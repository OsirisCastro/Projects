#define _WINSOCK_DEPRECATED_NO_WARNINGS
#define WIN32_LEAN_AND_MEAN
#define MAX_CLIENTS 3
#define MAX_USERS 3
#define NOMINMAX

#include "Server.h"
#include <winsock2.h>
#include <ws2tcpip.h>
#include <cstdint>
#include "definitions.h"
#include <iostream>
#include <string>
#include <thread>
#include <algorithm>
#include <sstream>
#include <windows.h>
#pragma comment(lib, "Ws2_32.lib")

// Sends all data
int tcp_send_whole(SOCKET skSocket, const char* data, uint16_t length) {
	int result, byteSent = 0;
	while (byteSent < length) {
		result = send(skSocket, data + byteSent, length - byteSent, 0);
		if (result <= 0) return result;
		byteSent += result;
	}
	return byteSent;
}

// Receives full data
int tcp_recv_whole(SOCKET s, char* buf, int len) {
	int total = 0;
	do {
		int ret = recv(s, buf + total, len - total, 0);
		if (ret < 1) return ret;
		total += ret;
	} while (total < len);
	return total;
}

int Server::init(uint16_t port)
{
	WSADATA wsadata;
	WSAStartup(WINSOCK_VERSION, &wsadata);

	this->listenSocket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	if (listenSocket == INVALID_SOCKET) return SETUP_ERROR;

	sockaddr_in serverAddr;
	serverAddr.sin_family = AF_INET;
	serverAddr.sin_addr.S_un.S_addr = INADDR_ANY;
	serverAddr.sin_port = htons(port);

	if (bind(listenSocket, (SOCKADDR*)&serverAddr, sizeof(serverAddr)) == SOCKET_ERROR)
		return BIND_ERROR;

	if (listen(listenSocket, 1) == SOCKET_ERROR)
		return SETUP_ERROR;

	std::thread([this, port]() { this->udpBroadcastThread(port); }).detach();

	return SUCCESS;
}

void Server::printHostInfo(uint16_t port)
{
	char hostname[256];
	if (gethostname(hostname, sizeof(hostname)) != 0) {
		std::cerr << "gethostname() failed\n";
		return;
	}

	std::cout << "Hostname: " << hostname << std::endl;

	struct addrinfo hints = {};
	struct addrinfo* res = nullptr;

	hints.ai_family = AF_UNSPEC; // both IPv4 and IPv6 are in there
	hints.ai_socktype = SOCK_STREAM;

	int status = getaddrinfo(hostname, nullptr, &hints, &res);
	if (status != 0) {
		std::cerr << "getaddrinfo failed: " << gai_strerrorA(status) << std::endl;
		return;
	}

	std::cout << "Resolved IP addresses:" << std::endl;

	for (struct addrinfo* p = res; p != nullptr; p = p->ai_next) {
		char ipstr[INET6_ADDRSTRLEN];

		void* addr;
		std::string ipver;

		if (p->ai_family == AF_INET) {
			struct sockaddr_in* ipv4 = (struct sockaddr_in*)p->ai_addr;
			addr = &(ipv4->sin_addr);
			ipver = "IPv4";
		}
		else if (p->ai_family == AF_INET6) {
			struct sockaddr_in6* ipv6 = (struct sockaddr_in6*)p->ai_addr;
			addr = &(ipv6->sin6_addr);
			ipver = "IPv6";
		}
		else {
			continue;
		}

		inet_ntop(p->ai_family, addr, ipstr, sizeof(ipstr));
		std::cout << "  [" << ipver << "] " << ipstr << ":" << port << std::endl;
	}

	freeaddrinfo(res);
}

int Server::readMessage(char* buffer, int32_t size)
{
	uint8_t msgSize = 0;
	int result = tcp_recv_whole(serverSocket, (char*)&msgSize, 1);
	if (result == 0) return SHUTDOWN;
	if (result == SOCKET_ERROR) return DISCONNECT;

	if (msgSize > size) return PARAMETER_ERROR;

	memset(buffer, 0, size);

	result = tcp_recv_whole(serverSocket, buffer, msgSize);
	if (result == 0) return SHUTDOWN;
	if (result == SOCKET_ERROR) return DISCONNECT;


	return SUCCESS;
}
int Server::sendMessage(SOCKET clientSocket, const char* data, int32_t length)
{
	if (length <= 0 || length > 255) return PARAMETER_ERROR;
	if (clientSocket == INVALID_SOCKET) return DISCONNECT;

	uint8_t size = (uint8_t)length;
	int result = tcp_send_whole(clientSocket, (char*)&size, 1);
	if (result == 0) return SHUTDOWN;
	if (result == SOCKET_ERROR) return DISCONNECT;

	result = tcp_send_whole(clientSocket, data, length);
	if (result == 0) return SHUTDOWN;
	if (result == SOCKET_ERROR) return DISCONNECT;

	return SUCCESS;
}

void Server::stop()
{
	shutdown(listenSocket, SD_BOTH);
	closesocket(listenSocket);
	shutdown(serverSocket, SD_BOTH);
	closesocket(serverSocket);
	WSACleanup();
}
void Server::stopClient()
{
	if (serverSocket != INVALID_SOCKET)
	{
		shutdown(serverSocket, SD_BOTH);
		closesocket(serverSocket);
		serverSocket = INVALID_SOCKET;
	}
}

void Server::multiplexLoop() {
	fd_set readfds;
	fd_set writefds;
	SOCKET max_fd = listenSocket;

	while (true) {
		FD_ZERO(&readfds);
		FD_ZERO(&writefds);

		FD_SET(listenSocket, &readfds);
		max_fd = listenSocket;

		for (SOCKET clientSocket : clientSockets) {
			FD_SET(clientSocket, &readfds);
			if (clientSocket > max_fd) {
				max_fd = clientSocket;
			}
		}

		struct timeval timeout = { 0, 0 };
		int activity = select((int)(max_fd + 1), &readfds, NULL, NULL, &timeout);

		if (activity < 0) {
			int err_code = WSAGetLastError();
			std::cerr << "Select error!" << std::endl;
			break;
		}

		// Handle new connections
		if (FD_ISSET(listenSocket, &readfds)) {
			sockaddr_in clientAddr;
			int addrLen = sizeof(clientAddr);
			SOCKET newClient = accept(listenSocket, (sockaddr*)&clientAddr, &addrLen);
			if (newClient != INVALID_SOCKET) {
				clientSockets.push_back(newClient);
				std::cout << "New client connected!" << std::endl;
				sendWelcomeMessage(newClient);
			}
		}

		// Handle client messages
		for (size_t i = 0; i < clientSockets.size(); ++i) {
			SOCKET clientSocket = clientSockets[i];

			if (FD_ISSET(clientSocket, &readfds)) {
				char buffer[1024];
				int bytesRead = recv(clientSocket, buffer, sizeof(buffer), 0);

				if (bytesRead <= 0) {
					std::cout << "Client disconnected" << std::endl;
					loggedInUsers.erase(clientSocket);
					closesocket(clientSocket);
					clientSockets.erase(clientSockets.begin() + i);
					--i;
					continue;
				}

				receiveBuffers[clientSocket].append(buffer, bytesRead);

				std::string& recvBuf = receiveBuffers[clientSocket];

				while (recvBuf.size() >= 1) {
					uint8_t len = static_cast<uint8_t>(recvBuf[0]);
					if (recvBuf.size() < 1 + len) {
						break;
					}

					std::string rawMessage = recvBuf.substr(1, len);
					recvBuf.erase(0, 1 + len);

					if (rawMessage.empty() || rawMessage.back() != '\0') {
						std::cerr << "Invalid message (missing null terminator)\n";
						continue;
					}

					std::string msg(rawMessage.c_str());

					std::cout << "[Client]: " << msg << std::endl;

					msg.erase(std::remove(msg.begin(), msg.end(), '\n'), msg.end());
					msg.erase(std::remove(msg.begin(), msg.end(), '\r'), msg.end());

					//Handle Client Commands and registration
					if (msg == "~help") {
						if (loggedInUsers.count(clientSocket)) {
							logCommand(loggedInUsers[clientSocket], msg);
						}
						sendFramedMessage(clientSocket, "Available Commands:\n");
						sendFramedMessage(clientSocket, "~help      - Show this help message\n");
						sendFramedMessage(clientSocket, "~getlist   - Shows a list of online clients\n");
						sendFramedMessage(clientSocket, "~logout    - Disconnects you from the Server\n");
						sendFramedMessage(clientSocket, "~login     - Login with username and password\n");
						sendFramedMessage(clientSocket, "~register  - Register your account <username> <password>\n");
						sendFramedMessage(clientSocket, "~send <username> <message> - Send private message\n");
					}
					else if (msg.rfind("~register", 0) == 0) {
						std::istringstream iss(msg);
						std::string cmd, username, password;
						iss >> cmd >> username >> password;

						if (username.empty() || password.empty()) {
							sendFramedMessage(clientSocket, "Usage: ~register <username> <password>\n");
						}
						else if (userCredentials.size() >= MAX_USERS) {
							sendFramedMessage(clientSocket, "Registration declined: server has reached user capacity.\n");
						}
						else if (userCredentials.find(username) != userCredentials.end()) {
							sendFramedMessage(clientSocket, "Registration failed: username already exists.\n");
						}
						else {
							userCredentials[username] = password;
							sendFramedMessage(clientSocket, "Registration successful! You may now log in.\n");
						}
					}
					else if (msg.rfind("~login", 0) == 0)
					{
						std::istringstream iss(msg);
						std::string cmd, username, password;
						iss >> cmd >> username >> password;

						if (username.empty() || password.empty()) {
							sendFramedMessage(clientSocket, "Usage: ~login <username> <password>\n");
						}
						else if (userCredentials.find(username) == userCredentials.end()) {
							sendFramedMessage(clientSocket, "Login failed: username not found.\n");
						}
						else if (userCredentials[username] != password) {
							sendFramedMessage(clientSocket, "Login failed: incorrect password.\n");
						}
						else if (std::find_if(loggedInUsers.begin(), loggedInUsers.end(),
							[&username](const std::pair<SOCKET, std::string>& p) { return p.second == username; }) != loggedInUsers.end())
						{
							sendFramedMessage(clientSocket, "Login failed: this user is already logged in elsewhere.\n");
						}
						else {
							loggedInUsers[clientSocket] = username;
							sendFramedMessage(clientSocket, "Login successful! Welcome, " + username + ".\n");
						}
					}
					else if (msg == "~getlist") {
						std::string userList = "Logged-in users:\n";
						for (const auto& pair : loggedInUsers) {
							userList += " - " + pair.second + "\n";
						}
						sendFramedMessage(clientSocket, userList);
					}
					else if (msg == "~logout") {
						sendFramedMessage(clientSocket, "Logging out. Goodbye!\n");
						loggedInUsers.erase(clientSocket);
						closesocket(clientSocket);
						clientSockets.erase(clientSockets.begin() + i);
						--i;
						continue;
					}
					else if (msg.rfind("~send ", 0) == 0) {
						std::istringstream iss(msg);
						std::string command, targetUser, content;
						iss >> command >> targetUser;
						std::getline(iss, content);
						if (!content.empty() && content[0] == ' ') content.erase(0, 1);

						auto it = std::find_if(loggedInUsers.begin(), loggedInUsers.end(),
							[&](const auto& pair) { return pair.second == targetUser; });

						if (it != loggedInUsers.end()) {
							SOCKET targetSocket = it->first;
							std::string sender = loggedInUsers[clientSocket];
							std::string messageToSend = "[Private] " + sender + ": " + content + "\n";
							sendFramedMessage(targetSocket, messageToSend);
							sendFramedMessage(clientSocket, "Message sent to " + targetUser + "\n");
						}
						else {
							sendFramedMessage(clientSocket, "User '" + targetUser + "' is not online.\n");
						}
					}
					else if (msg == "~getlog") {
						logCommand(loggedInUsers[clientSocket], msg);
						std::ifstream logFile(PUBLIC_LOG_FILE);
						if (!logFile) {
							sendFramedMessage(clientSocket, "No public messages log found.\n");
						}
						else {
							std::string line;
							sendFramedMessage(clientSocket, "Public Message Log:\n");
							while (std::getline(logFile, line)) {
								sendFramedMessage(clientSocket, line + "\n");
							}
						}
					}
					else {
						// Broadcast to everyone else
						if (loggedInUsers.count(clientSocket)) {
							std::string sender = loggedInUsers[clientSocket];
							std::string broadcastMsg = sender + ": " + msg + "\n";

							//log messages
							logPublicMessage(sender, msg);

							for (SOCKET sock : clientSockets) {
								if (sock != clientSocket && loggedInUsers.count(sock)) {
									sendFramedMessage(sock, broadcastMsg);
								}
							}
						}
						else {
							sendFramedMessage(clientSocket, "You must be logged in to send messages.\n");
						}
					}
				}
			}
		}
	}
}

void Server::relayMessageToOthers(SOCKET senderSocket, const char* data)
{
	for (SOCKET clientSocket : clientSockets) {
		if (clientSocket != senderSocket) {
			sendFramedMessage(clientSocket, data);
		}
	}
}

int Server::sendFramedMessage(SOCKET clientSocket, const std::string& data) {
	return sendMessage(clientSocket, data.c_str(), static_cast<int>(data.length() + 1));
}

void Server::sendWelcomeMessage(SOCKET clientSocket) {

	std::string welcomeMsg = "Welcome to the Chat Server! Use ' ~ ' for commands and '~help' to see all commands available\n";
	int sendResult = sendMessage(clientSocket, (char*)welcomeMsg.c_str(), static_cast<int32_t>(welcomeMsg.length() + 1));
	if (sendResult != SUCCESS) {
		std::cerr << "[Failed to send welcome message]\n";
	}
}

void Server::logPublicMessage(const std::string& username, const std::string& message) {
	std::ofstream outFile("PublicMessages.log", std::ios::app);
	if (outFile.is_open()) {
		outFile << message << std::endl;
	}
}

void Server::udpBroadcastThread(uint16_t tcpPort) {

	std::cout << "[Thread] UDP Broadcast thread started on port " << tcpPort << std::endl;

	SOCKET udpSocket = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
	if (udpSocket == INVALID_SOCKET) {
		std::cerr << "Failed to create UDP socket.\n";
		return;
	}

	// Enable broadcast
	BOOL broadcast = TRUE;
	if (setsockopt(udpSocket, SOL_SOCKET, SO_BROADCAST, (char*)&broadcast, sizeof(broadcast)) == SOCKET_ERROR) {
		std::cerr << "Failed to set broadcast option.\n";
		closesocket(udpSocket);
		return;
	}

	sockaddr_in broadcastAddr;
	broadcastAddr.sin_family = AF_INET;
	broadcastAddr.sin_port = htons(31337);
	broadcastAddr.sin_addr.S_un.S_addr = INADDR_BROADCAST;

	// Get IP address
	char hostname[256];
	gethostname(hostname, sizeof(hostname));
	addrinfo hints{}, * res = nullptr;
	hints.ai_family = AF_INET;
	hints.ai_socktype = SOCK_STREAM;

	if (getaddrinfo(hostname, NULL, &hints, &res) != 0 || res == nullptr) {
		std::cerr << "Failed to get local IP.\n";
		closesocket(udpSocket);
		return;
	}

	char ipStr[INET_ADDRSTRLEN];
	sockaddr_in* ipv4 = (sockaddr_in*)res->ai_addr;
	inet_ntop(AF_INET, &ipv4->sin_addr, ipStr, sizeof(ipStr));
	freeaddrinfo(res);

	std::string message = std::string("ServerIP: ") + ipStr + " Port: " + std::to_string(tcpPort);

	while (true) {
		int sent = sendto(udpSocket, message.c_str(), (int)message.size(), 0,
			(sockaddr*)&broadcastAddr, sizeof(broadcastAddr));
		if (sent == SOCKET_ERROR) {
			std::cerr << "UDP broadcast failed.\n";
		}
		std::this_thread::sleep_for(std::chrono::seconds(5));  // Broadcast time
	}

	closesocket(udpSocket);
}

void Server::logCommand(const std::string& username, const std::string& command) {
	std::ofstream outFile("UserCommands.log", std::ios::app);
	if (outFile.is_open()) {
		outFile << username << ": " << command << std::endl;
	}
}

