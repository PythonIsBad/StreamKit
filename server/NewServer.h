#pragma once
#include <SFML/Network.hpp>
#include <thread>
#include <map>
#include "Room.h"
#include "Funcs.h"
#include "Logs.h"

const int basicDataSize = 65000;

class Server
{
private:
    std::thread receiveThread;
public:
    std::atomic <bool> running = false;
    unsigned short listenPort;
    sf::UdpSocket receiveSocket, sendSocket;
    sf::UdpSocket tmpSocket;
    std::atomic <bool> useSpecialIP = false;
    sf::IpAddress specialIP;
public:
    Server();
    void start(unsigned short LISTEN_PORT);
    void send(long long ROOM_ID, long long CLIENT_ID, std::string MESSAGE);
    void sendIdentic(long long ROOM_ID, long long CLIENT_ID, std::string MESSAGE, int receivedSize);
    void sendIdentic2(long long ROOM_ID, long long CLIENT_ID, char * MESSAGE, int receivedSize);
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

                logSpecial("received size: " + intToString(receivedSize));
                int userID = receivedData[6];
                int roomID = receivedData[7];
                Room* room;
                if (rooms.find(roomID) != rooms.end()) room = rooms[roomID];
                else continue;
                User* streamer;
                if (room->users.find(userID) != room->users.end())
                    streamer = room->users[userID];
                else continue;
                logSpecial("trying to send stream from " + intToString(userID) + " in room " + intToString(roomID));
                for (auto it = room->users.begin(); it != room->users.end(); ++it)
                {
                    if (it->second->watching)
                    {
                        server->sendIdentic2(roomID, it->first, receivedData, receivedSize);
                        logSpecial("Server: packet sended from " + intToString(userID) + " to " + intToString(it->first) + " in room " + intToString(roomID));
                    }
                }
                continue;
            }

            decodedData = decodeData(receivedData, receivedSize);
            logS("Server receive thread receive data: "
                + intToString(decodedData.size()) + " elements");

            if (server->useSpecialIP)
                receivedIp = server->specialIP;

            //decodedData.erase(decodedData.begin() + 1);

            for (int i = 0; i < decodedData.size(); i++)
                logS(intToString(i) + ": [" + decodedData[i] + "]");
            if (receivedIp == sf::IpAddress("127.0.0.1") && decodedData[0] == "EXIT")
                break;

            if (decodedData[0] == "CREATE_ROOM")
            {
                long long roomCode;
                if (!freeRooms.empty())
                {
                    roomCode = freeRooms.front();
                    freeRooms.pop();
                }
                else
                {
                    roomCode = maxRoomID;
                    maxRoomID++;
                }
                if (rooms.find(roomCode) == rooms.end())
                    rooms.insert({ roomCode, new Room() });
                std::string ans = "ROOM_CREATED\n" + intToString(roomCode);
                server->tmpSocket.send(ans.c_str(), ans.size(), receivedIp, stringToInt(decodedData[1]));
                logS("Server: room " + intToString(roomCode) + " created");
            }
            else if (decodedData[0] == "CONNECTION_REQUEST")
            {
                std::string ans = "CONNECTION_ACCEPTED";
                server->tmpSocket.send(ans.c_str(), ans.size(), receivedIp, stringToInt(decodedData[1]));
                logS("Server: connection accepted");
            }
            else if (decodedData[0] == "GET_ACTIVE_ROOMS")
            {
                std::string ans = "ACTIVE_ROOMS_PRESENT\n" + intToString(rooms.size());
                for (auto it = rooms.begin(); it != rooms.end(); ++it)
                    ans += "\n" + std::to_string(it->first);
                server->tmpSocket.send(ans.c_str(), ans.size(), receivedIp, stringToInt(decodedData[1]));


                for (auto it = rooms.begin(); it != rooms.end(); ++it)
                {
                    std::cout << "room " << it->first << "\n";
                    for (auto it1 = it->second->users.begin(); it1 != it->second->users.end(); ++it1)
                        std::cout << "user: " << it1->first << "\n";
                }

                logS("Server: active rooms list sended");
            }
            else if (decodedData[0] == "CLOSE_ROOM")
            {
                long long roomCode = stringToInt(decodedData[2]);
                freeRooms.push(roomCode);
                std::string ans = "ROOM_CLOSED";
                if (rooms.find(roomCode) != rooms.end())
                {
                    for (auto it = rooms[roomCode]->users.begin(); it != rooms[roomCode]->users.end(); ++it)
                        server->tmpSocket.send(ans.c_str(), ans.size(), it->second->sessionIp, it->second->sessionPort);
                    delete rooms[roomCode];
                    rooms.erase(roomCode);
                }
                else
                {
                    ans = "SUCH_ROOM_NOT_EXIST\n" + decodedData[2];
                    server->tmpSocket.send(ans.c_str(), ans.size(), receivedIp, stringToInt(decodedData[1]));
                    continue;
                }
                logS("Server: room " + decodedData[2] + " closed");
            }
            else if (decodedData[0] == "CONNECT_USER")
            {
                sf::IpAddress userIp = receivedIp;
                unsigned short userPort = stringToInt(decodedData[1]);
                long long roomCode = stringToInt(decodedData[2]);
                long long userCode = stringToInt(decodedData[3]);
                Room* room;
                std::string ans;
                if (rooms.find(roomCode) != rooms.end()) room = rooms[roomCode];
                else
                {
                    ans = "SUCH_ROOM_NOT_EXIST\n" + decodedData[2];
                    server->tmpSocket.send(ans.c_str(), ans.size(), receivedIp, stringToInt(decodedData[1]));
                    continue;
                }
                if (room->users.find(userCode) == room->users.end())
                    room->users.insert({ userCode, new User(userIp, userPort) });
                else
                {
                    ans = "YOU_ALREADY_CONNECTED\n" + decodedData[2];
                    server->tmpSocket.send(ans.c_str(), ans.size(), receivedIp, stringToInt(decodedData[1]));
                    continue;
                }
                ans = "YOU_CONNECTED\n" + decodedData[2];
                server->tmpSocket.send(ans.c_str(), ans.size(), receivedIp, stringToInt(decodedData[1]));
                logS("Server: user " + decodedData[3] + " connected to room " + decodedData[2]);
            }
            else if (decodedData[0] == "DISCONNECT_USER")
            {
                long long roomCode = stringToInt(decodedData[1]);
                long long userCode = stringToInt(decodedData[2]);
                Room* room;
                std::string ans;
                if (rooms.find(roomCode) != rooms.end()) room = rooms[roomCode];
                else
                {
                    ans = "SUCH_ROOM_NOT_EXIST\n" + decodedData[2];
                    server->tmpSocket.send(ans.c_str(), ans.size(), receivedIp, stringToInt(decodedData[1]));
                    continue;
                }
                if (room->users.find(userCode) != room->users.end())
                {
                    ans = "YOU_DISCONNECTED";
                    server->tmpSocket.send(ans.c_str(), ans.size(), receivedIp, room->users[userCode]->sessionPort);
                    delete room->users[userCode];
                    room->users.erase(userCode);
                }
                else
                {
                    ans = "YOU_ALREADY_DISCONNECTED\n" + decodedData[2];
                    server->tmpSocket.send(ans.c_str(), ans.size(), receivedIp, stringToInt(decodedData[1]));
                    continue;
                }
                logS("Server: user " + decodedData[2] + " disconnected from room " + decodedData[1]);
            }
            else if (decodedData[0] == "CONNECT_TO_STREAM")
            {
                long long roomCode = stringToInt(decodedData[1]);
                long long listenCode = stringToInt(decodedData[2]);
                Room* room;
                if (rooms.find(roomCode) != rooms.end()) room = rooms[roomCode];
                else continue; // such room not exist
                User* listener;
                if (room->users.find(listenCode) != room->users.end())
                    listener = room->users[listenCode];
                else continue;
                listener->watching = true;
                std::string ans = "YOU_CONNECTED_TO_STREAM";
                server->tmpSocket.send(ans.c_str(), ans.size(), receivedIp, room->users[listenCode]->sessionPort);

                for (auto it = room->users.begin(); it != room->users.end(); ++it)
                {
                    if (it->second->streaming)
                    {
                        ans = "RESET_STREAM";
                        server->tmpSocket.send(ans.c_str(), ans.size(), it->second->sessionIp, it->second->sessionPort);
                        break;
                    }
                }
                logS("Server: user " + decodedData[2] + " connected to stream in room " + decodedData[1]);
            }
            else if (decodedData[0] == "DISCONNECT_FROM_STREAM")
            {
                long long roomCode = stringToInt(decodedData[1]);
                long long listenCode = stringToInt(decodedData[2]);
                Room* room;
                if (rooms.find(roomCode) != rooms.end()) room = rooms[roomCode];
                else continue; // such room not exist
                User* listener;
                if (room->users.find(listenCode) != room->users.end())
                    listener = room->users[listenCode];
                else continue;
                listener->watching = false;
                std::string ans = "YOU_DISCONNECTED_TO_STREAM";
                server->tmpSocket.send(ans.c_str(), ans.size(), receivedIp, room->users[listenCode]->sessionPort);
                logS("Server: user " + decodedData[2] + " disconnected from stream in room " + decodedData[1]);
            }
            else if (decodedData[0] == "START_STREAM")
            {
                long long roomCode = stringToInt(decodedData[1]);
                long long streamerCode = stringToInt(decodedData[2]);
                std::string ans = "STREAM_STARTED\n" + decodedData[2];
                Room* room;
                if (rooms.find(roomCode) != rooms.end()) room = rooms[roomCode];
                else continue; // such room not exist
                for (auto it = room->users.begin(); it != room->users.end(); ++it)
                    if(it->first != streamerCode)
                        server->tmpSocket.send(ans.c_str(), ans.size(), it->second->sessionIp, it->second->sessionPort);
                room->users[streamerCode]->watching = false;
            }
        }
    }
    logS("Server receive thread stopped");
}

Server::Server()
{
}

void Server::start(unsigned short LISTEN_PORT)
{
    logS("Server starting...");
    if (running)
    {
        logS("Server error: already started");
        return;
    }
    listenPort = LISTEN_PORT;
    if (receiveSocket.bind(listenPort) != sf::Socket::Done)
    {
        logS("Server error: cannot bind socket");
        return;
    }
    listenPort = receiveSocket.getLocalPort();
    logS("Server running on port " + intToString(listenPort));
    receiveThread = std::thread(ServerReceiveProc, this);
    running = true;
    logS("Server started");
}

void Server::send(long long ROOM_ID, long long CLIENT_ID, std::string MESSAGE)
{
    MESSAGE += '\\';
    if (rooms.find(ROOM_ID) != rooms.end())
    {
        Room* room = rooms[ROOM_ID];
        if (room->users.find(CLIENT_ID) != room->users.end())
        {
            User* user = room->users[CLIENT_ID];
            sendSocket.send(MESSAGE.c_str(), MESSAGE.size(), user->sessionIp, user->sessionPort);
        }
        else
            logS("Server error: not exist client " + intToString(CLIENT_ID));
    }
    else
        logS("Server error: not exist room " + intToString(ROOM_ID));
}

void Server::sendIdentic(long long ROOM_ID, long long CLIENT_ID, std::string MESSAGE, int SIZE)
{
    if (rooms.find(ROOM_ID) != rooms.end())
    {
        Room* room = rooms[ROOM_ID];
        if (room->users.find(CLIENT_ID) != room->users.end())
        {
            User* user = room->users[CLIENT_ID];
            sendSocket.send(MESSAGE.c_str(), SIZE, user->sessionIp, user->sessionPort);
        }
        else
            logS("Server error: not exist client " + intToString(CLIENT_ID));
    }
    else
        logS("Server error: not exist room " + intToString(ROOM_ID));
}

void Server::sendIdentic2(long long ROOM_ID, long long CLIENT_ID, char* MESSAGE, int SIZE)
{
    if (rooms.find(ROOM_ID) != rooms.end())
    {
        Room* room = rooms[ROOM_ID];
        if (room->users.find(CLIENT_ID) != room->users.end())
        {
            User* user = room->users[CLIENT_ID];
            sendSocket.send(MESSAGE, SIZE, user->sessionIp, user->sessionPort);
        }
        else
            logS("Server error: not exist client " + intToString(CLIENT_ID));
    }
    else
        logS("Server error: not exist room " + intToString(ROOM_ID));
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
    receiveThread.join();
    Room* room;
    for (auto roomIt : rooms)
    {
        room = roomIt.second;
        /*for (auto userIt : room->users)
        {
            send(roomIt.first, userIt.first, "DISCONNECT");
        }*/
        roomIt.second->users.clear();
        delete roomIt.second;
    }
    rooms.clear();
    logS("Server stopped");
}

Server::~Server()
{
    stop();
}

//sf::IpAddress sendIP("192.168.2.181");