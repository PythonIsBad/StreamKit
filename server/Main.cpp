#include <iostream>
#include <string>
#include <map>
#include <vector>
#include <SFML/Network.hpp>
#include "Logs.h"
#include "Funcs.h"
#include "Room.h"
#include "NewServer.h"

int main()
{
	doLog = true;
	resetLog();
	Server* server = new Server();
	server->useSpecialIP = false;
	server->specialIP = sf::IpAddress("192.168.2.181");
	//server->specialIP = sf::IpAddress("25.41.207.37");
	server->start(50210);
	std::string cmd;
	while (1)
	{
		std::cin >> cmd;
		if (cmd == "help")
		{
			std::cout << "==== COMMAND LIST ====\n";
			std::cout << "         help\n";
			std::cout << "      log_enable\n";
			std::cout << "      log_disable\n";
			std::cout << "         exit\n";
			std::cout << "== COMMAND LIST END ==\n";
		}
		else if (cmd == "log_enable")
		{
			logEnabled = true;
			std::cout << "Logs enabled to terminal\n";
		}
		else if(cmd == "log_disable")
		{
			logEnabled = false;
			std::cout << "Logs disabled from terminal\n";
		}
		else if (cmd == "exit")
		{
			break;
		}
		else
		{
			std::cout << "Not expectable command! Print \'help\' to check command list\n";
		}
	}
	server->stop();
	delete server;
	return 0;
}