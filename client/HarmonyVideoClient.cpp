#include <iostream>
#include "Logs.h"
#include "Server.h"
#include "Compressor.h"
#include "Decompressor.h"
#include "UserID.h"

void streamProc(Server* server)
{
	while (1)
	{
		if (streaming)
		{
			compressor->runCompression();
			for (int i = 0; i < 10; i++)
			{
				server->sendBytes(compressor->newScreen->packets[i]->encodedData, compressor->newScreen->packets[i]->encodedSize);
				Sleep(1);
			}
		}
		Sleep(50);
	}
}

void commandProc(Server* server)
{
	std::string cmd = "";
	while (1)
	{
		std::cin >> cmd;
		if (cmd == "exit")
			break;
		else if (cmd == "help")
		{
			std::cout << "== Command list: ==\n";
			std::cout << "connection_request\n";
			std::cout << "get_active_rooms\n";
			std::cout << "create_room\n";
			std::cout << "close_room [id]\n";
			std::cout << "connect_user [room_id]\n";
			std::cout << "disconnect_user\n";
			std::cout << "connect_to_stream\n";
			std::cout << "disconnect_from_stream\n";
			std::cout << "start_stream\n";
			std::cout << "stop_stream\n";
			std::cout << "== Command list end ==\n";
		}
		else if (cmd == "connection_request")
		{
			server->send("CONNECTION_REQUEST\n" + intToString(server->listenPort));
		}
		else if (cmd == "get_active_rooms")
		{
			server->send("GET_ACTIVE_ROOMS\n" + intToString(server->listenPort));
		}
		else if (cmd == "create_room")
		{
			server->send("CREATE_ROOM\n" + intToString(server->listenPort));
		}
		else if (cmd == "close_room")
		{
			int roomCode;
			std::cout << "Print room nubmer: ";
			std::cin >> roomCode;
			server->send("CLOSE_ROOM\n" + intToString(server->listenPort) + "\n" + intToString(roomCode));
		}
		else if (cmd == "connect_user")
		{
			int roomCode;
			std::cout << "Print room nubmer: ";
			std::cin >> roomCode;
			server->send("CONNECT_USER\n" + intToString(server->listenPort) + "\n" + intToString(roomCode) + "\n" + intToString(userID));
		}
		else if (cmd == "disconnect_user")
		{
			server->send("DISCONNECT_USER\n" + intToString(roomID) + "\n" + intToString(userID));
			roomID = -1;
			streaming = false;
		}
		else if (cmd == "connect_to_stream")
		{
			if (roomID == -1)
			{
				std::cout << "You not in the room!\n";
				continue;
			}
			if (streaming)
			{
				std::cout << "You are streamer!\n";
				continue;
			}
			server->send("CONNECT_TO_STREAM\n" + intToString(roomID) + "\n" + intToString(userID));
		}
		else if (cmd == "disconnect_from_stream")
		{
			if (roomID == -1)
			{
				std::cout << "You not in the room!\n";
				continue;
			}
			server->send("DISCONNECT_FROM_STREAM\n" + intToString(roomID) + "\n" + intToString(userID));
		}
		else if (cmd == "start_stream")
		{
			if (roomID == -1)
			{
				std::cout << "You not in the room!\n";
				continue;
			}
			server->send("START_STREAM\n" + intToString(roomID) + "\n" + intToString(userID));
			streaming = true;
		}
		else if (cmd == "stop_stream")
		{
			if (roomID == -1)
			{
				std::cout << "You not in the room!\n";
				continue;
			}
			server->send("STOP_STREAM\n" + intToString(roomID) + "\n" + intToString(userID));
			streaming = false;
		}
		else
		{
			std::cout << "Not expectable command. Print \'help\' to see commands list.\n";
		}
	}
}

int main()
{
	screenSizeX = 600;
	screenSizeY = 400;
	doLog = true;
	unsigned short serverPort = 50210;
	std::cout << "Print your id: ";
	int user_id;
	std::cin >> user_id;
	userID = user_id;
	std::cout << "Print server port: ";
	std::cin >> serverPort;
	resetLog(userID);
	window = new sf::RenderWindow();
	window->create(sf::VideoMode(screenSizeX, screenSizeY), L"Client window", sf::Style::Default);
	compressor = new Compressor();
	decompressor = new Decompressor();
	Server* server = new Server();
	server->targetPort = serverPort;
	server->start(0, userID);
	HANDLE streamThread, commandThread;
	streamThread = CreateThread((LPSECURITY_ATTRIBUTES)NULL, 0, (LPTHREAD_START_ROUTINE)streamProc, server, 0, NULL);
	commandThread = CreateThread((LPSECURITY_ATTRIBUTES)NULL, 0, (LPTHREAD_START_ROUTINE)commandProc, server, 0, NULL);

	sf::Event event;
	while (window != nullptr)
	{
		while (window->pollEvent(event))
		{
			//
		}
		window->draw(decompressor->sprite);
		window->display();
		Sleep(10);
	}
	server->stop();
	window = nullptr;
	delete server;
	return 0;
}