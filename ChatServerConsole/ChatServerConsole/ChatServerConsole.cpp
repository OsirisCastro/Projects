// ChatServerConsole.cpp : This file contains the 'main' function. Program execution begins and ends there.
//

#include <iostream>
#include <string>
#include "Server.h"

int main()
{
	std::string choice;
	do
	{
		std::cout << "Create Server - Press 1:\n>";
		std::cin >> choice;
	} while (choice != "1");

	if (choice == "1")
	{
		uint16_t port = 0;
		std::string portPresets;
		std::cout << "\nChoose a port preset or type your own:\n";
		std::cout << "1 - Use port 31337\n";
		std::cout << "2 - Enter Port Number: \n";
		std::cout << "> ";
		std::cin >> portPresets;

		if (portPresets == "1")
		{
			port = 31337;
		}
		else if (portPresets == "2")
		{
			std::cout << "Enter Port Number : \n";
			std::cin >> port;
		}
		else
		{
			std::cerr << "Invalid, Exiting\n";
			return 1;
		}

		std::cout << "Listening...\n";
		Server server;
		int result = server.init(port);

		if (result == SUCCESS)
		{
			std::cout << "Server Started Successfully!\n";
			server.printHostInfo(port);
			server.multiplexLoop();
		}
		else
		{
			std::cerr << "Failed to initialize Server! Error: " << result << "\n";
		}

		server.stop();
	}

	return 0;

}