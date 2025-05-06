#pragma once
#include <string>
#include <vector>
#include <set>
#include <SFML/Network.hpp>

struct User
{
	sf::IpAddress sessionIp;
	unsigned short sessionPort;
	bool watching = false;
	bool streaming = false;
	User(sf::IpAddress IP, unsigned short PORT)
	{
		sessionIp = IP;
		sessionPort = PORT;
	}
};