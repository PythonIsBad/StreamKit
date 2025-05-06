#pragma once
#include <SFML/Network.hpp>
#include <Windows.h>
#include <map>
#include "Funcs.h"
#include "Logs.h"
#include "UserID.h"
#include "Compressor.h"
#include "Decompressor.h"

const int basicDataSize = 65000;

class Server
{
private:
    HANDLE receiveThread;
public:
    sf::IpAddress targetIp;
    unsigned short targetPort;
    bool running = false;
    unsigned short listenPort;
    sf::UdpSocket receiveSocket, sendSocket;

    sf::UdpSocket tmpSocket;
    unsigned short tmpPort;
public:
    Server();
    void start(unsigned short LISTEN_PORT, int id);
    void send(std::string MESSAGE);
    void sendBytes(char * MESSAGE, int size);
    void stop();
    ~Server();
};

void ServerReceiveProc(Server* server)
{
    logS("Server receive thread started");
    char receivedData[basicDataSize];
    sf::IpAddress receivedIp;
    unsigned short receivedPort;
    std::size_t receivedSize;
    std::vector <std::string> decodedData;
    server->send("CONNECTION_REQUEST\n" + intToString(server->listenPort));
    while (1)
    {
        if (server->receiveSocket.receive(receivedData, basicDataSize, receivedSize, receivedIp, receivedPort) != sf::Socket::Done)
        {
            logS("Server receive thread error: cannot receive data");
            continue;
        }
        else
        {
            if (receivedData[0] == 'S' &&
                receivedData[1] == 'T' &&
                receivedData[2] == 'R' &&
                receivedData[3] == 'E' &&
                receivedData[4] == 'A' &&
                receivedData[5] == 'M')
            {
                //std::cout << "received size: " << receivedSize << "\n";
                for (int i = 0; i < receivedSize; i++)
                    decompressor->screen->packets[receivedData[8]]->encodedData[i] = receivedData[i];
                if (receivedData[8] == 9)
                    decompressor->runDecompression();
                continue;
            }
            decodedData = decodeData(receivedData, receivedSize);
            logS("Server receive thread receive data: "
                + intToString(decodedData.size()) + " elements");
            for (int i = 0; i < decodedData.size(); i++)
                logS(intToString(i) + ": [" + decodedData[i] + "]");
            if (receivedIp == sf::IpAddress("127.0.0.1") && decodedData[0] == "EXIT")
                break;

            if (decodedData[0] == "YOU_CONNECTED")
            {
                roomID = stringToInt(decodedData[1]);
            }
            else if (decodedData[0] == "ROOM_CLOSED")
            {
                roomID = -1;
                streaming = false;
            }
            else if (decodedData[0] == "RESET_STREAM")
            {
                compressor->resetStream = true;
            }
            else if (decodedData[0] == "STREAM_STARTED")
            {
                streaming = false;
                std::cout << "User " << decodedData[1] << " started stream\n";
            }
        }
    }
    logS("Server receive thread stopped");
}

Server::Server()
{
    receiveThread = NULL;
}

void Server::start(unsigned short LISTEN_PORT, int id)
{
    logS("Server starting...");
    std::ifstream fin("server_ip.txt");
    fin >> targetIp;
    fin.close();
    if (running)
    {
        logS("Server error: already started");
        return;
    }
    listenPort = 55000 + id;
    if (receiveSocket.bind(listenPort) != sf::Socket::Done)
    {
        logS("Server error: cannot bind socket");
        return;
    }
    listenPort = receiveSocket.getLocalPort();
    logS("Server choose port " + intToString(listenPort));
    receiveThread = CreateThread((LPSECURITY_ATTRIBUTES)NULL, 0, (LPTHREAD_START_ROUTINE)ServerReceiveProc, this, 0, NULL);
    running = true;
    Sleep(100);
    logS("Server started");
}

void Server::send(std::string MESSAGE)
{
    MESSAGE += '\\';
    sendSocket.send(MESSAGE.c_str(), MESSAGE.size(), targetIp, targetPort);
}

void Server::sendBytes(char* message, int size)
{
    sendSocket.send(message, size, targetIp, targetPort);
}

void Server::stop()
{
    logS("Server stopping...");
    if (!running)
    {
        logS("Server error: already stopped");
        return;
    }
    running = false;
    sf::UdpSocket exitSocket;
    exitSocket.send("EXIT", 4, sf::IpAddress("127.0.0.1"), listenPort);
    WaitForSingleObject(receiveThread, INFINITE);
    CloseHandle(receiveThread);
    receiveThread = NULL;
    logS("Server stopped");
}

Server::~Server()
{
    stop();
}