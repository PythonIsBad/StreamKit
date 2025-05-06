#pragma once
#include "SFML/Graphics.hpp"
#include <atomic>

sf::RenderWindow* window;
std::atomic <int> userID = 0;
std::atomic <int> roomID = -1;
std::atomic <bool> streaming = false;