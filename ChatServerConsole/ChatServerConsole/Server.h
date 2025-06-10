#pragma once
#include "platform.h"
#include "definitions.h"
#include <vector>
#include <unordered_map>
#include <string>
#include <fstream>

class Server
{
public:

	int init(uint16_t port);
	void printHostInfo(uint16_t port);
	int readMessage(char* buffer, int32_t size);
	int sendMessage(SOCKET clientSocket, const char* data, int32_t length);
	void stop();
	void stopClient();
	void sendWelcomeMessage(SOCKET clientSocket);
	void multiplexLoop();
	void relayMessageToOthers(SOCKET senderSocket, const char* message);
	int sendFramedMessage(SOCKET clientSocket, const std::string& message);
	void logCommand(const std::string& username, const std::string& command);
	void logPublicMessage(const std::string& username, const std::string& message);
	void udpBroadcastThread(uint16_t tcpPort);

private:
	std::unordered_map<std::string, std::string> userCredentials;
	std::unordered_map<SOCKET, std::string> loggedInUsers;
	std::unordered_map<SOCKET, std::string> receiveBuffers;
	std::unordered_map<SOCKET, std::string> socketToUsername;
	std::unordered_map<std::string, SOCKET> usernameToSocket;
	SOCKET listenSocket = INVALID_SOCKET;
	SOCKET serverSocket = INVALID_SOCKET;
	std::vector<SOCKET> clientSockets;
	const std::string COMMAND_LOG_FILE = "UserCommands.log";
	const std::string PUBLIC_LOG_FILE = "PublicMessages.log";
};