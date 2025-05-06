#pragma once
#include <vector>
#include <queue>
#include <map>
#include "User.h"

struct Room
{
	std::map <long long, User*> users;
	User* owner = nullptr;
	Room() { }
	~Room()
	{
		for (auto it = users.begin(); it != users.end(); ++it)
			delete it->second;
		users.clear();
	}
};

std::map <long long, Room*> rooms;
std::queue <int> freeRooms;
int maxRoomID = 1;