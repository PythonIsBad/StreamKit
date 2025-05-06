#pragma once

#include <iostream>
#include "SFML/Graphics.hpp"
#include <Windows.h>
#include "ScreenBase.h"

class Decompressor
{
private:
	int smallXSize = 1920, smallYSize = 1080;
	int shaderXSize = 1920, shaderYSize = 1088;
	unsigned char* rawPixelQuadData;
	int bytePos, n, loadPhase, emptyCounter;
	double tmpDouble;
	DCTBlock* tmpDCTBlock;
	StablePacket* tmpPacket;
	sf::Texture rawTexture;
	sf::Sprite rawSprite;
	sf::Shader shader;
	sf::RenderTexture rawRenderTexture;
	sf::Texture texture;
	int sumBytes;
public:
	Screen* screen;
	sf::Sprite sprite;
public:
	Decompressor()
	{
		rawPixelQuadData = (unsigned char*)(malloc(shaderXSize * shaderYSize * 4));
		for (int i = 0; i < shaderXSize * shaderYSize; i++)
		{
			rawPixelQuadData[i * 4] = 0;
			rawPixelQuadData[i * 4 + 3] = 255;
		}
		for (int x = shaderXSize / downsamplingSize; x < shaderXSize; x++)
		{
			for (int y = shaderYSize / downsamplingSize; y < shaderYSize; y++)
			{
				rawPixelQuadData[(x + y * shaderXSize) * 4 + 1] = 0;
				rawPixelQuadData[(x + y * shaderXSize) * 4 + 2] = 0;
			}
		}
		rawTexture.create(shaderXSize, shaderYSize);
		rawSprite.setTexture(rawTexture);
		rawRenderTexture.create(shaderXSize, shaderYSize);
		texture.create(shaderXSize, shaderYSize);
		sprite.setTexture(texture);
		sprite.setScale(screenSizeX * 1. / fullXSize, screenSizeY * 1. / fullYSize);
		shader.loadFromFile("shaders/stable_decompress_shader.frag", sf::Shader::Fragment);
		shader.setUniform("screen_resolution", sf::Vector2f(shaderXSize, shaderYSize));
		screen = new Screen();
	}
	void runDecompression()
	{
		countDecompress();
		rawTexture.update(rawPixelQuadData);
		rawRenderTexture.draw(rawSprite, &shader);
		rawRenderTexture.display();
		texture.update(rawRenderTexture.getTexture());
	}
private:
	void countDecompress()
	{
		sumBytes = 0;
		for (int packetI = 0; packetI < 10; packetI++)
		{
			tmpPacket = screen->packets[packetI];
			bytePos = 9;
			for (int blockI = 0; blockI < 212; blockI++)
			{
				for (int DCTBlockI = 0; DCTBlockI < 18; DCTBlockI++)
				{
					if (DCTBlockI == 16)
						tmpDCTBlock = &(tmpPacket->blocks[blockI].colorBlockCb);
					else if (DCTBlockI == 17)
						tmpDCTBlock = &(tmpPacket->blocks[blockI].colorBlockCr);
					else
						tmpDCTBlock = &(tmpPacket->blocks[blockI].brightBlock[DCTBlockI]);
					if (tmpPacket->encodedData[bytePos] == 127)
					{
						bytePos++;
						continue;
					}
					n = tmpPacket->encodedData[bytePos] / 4;
					loadPhase = tmpPacket->encodedData[bytePos] % 4;
					bytePos++;
					emptyCounter = loadPhase * 16;
					for (int i = 0; i < 8; i++)
					{
						for (int j = 0; j <= i; j++)
						{
							if (emptyCounter > 0)
								emptyCounter--;
							else if (n > 0)
							{
								tmpDCTBlock->data[i - j + 8 * j] = tmpPacket->encodedData[bytePos] + 128;
								bytePos++;
								n--;
							}
							else
								tmpDCTBlock->data[i - j + 8 * j] = 128;
						}
					}
					for (int i = 1; i < 8; i++)
					{
						for (int j = 0; j < 8 - i; j++)
						{
							if (emptyCounter > 0)
								emptyCounter--;
							else if (n > 0)
							{
								tmpDCTBlock->data[7 - j + 8 * (i + j)] = tmpPacket->encodedData[bytePos] + 128;
								bytePos++;
								n--;
							}
							else
								tmpDCTBlock->data[7 - j + 8 * (i + j)] = 128;
						}
					}
				}
			}
			//std::cout << "Packet " << packetI << ": " << bytePos << "\n";
			sumBytes += bytePos;
		}

		for (int y = 0; y < 34; y++)
		{
			for (int x = 0; x < 60; x++)
			{
				for (int smallY = 0; smallY < 32; smallY++)
				{
					for (int smallX = 0; smallX < 32; smallX++)
					{
						rawPixelQuadData[(x * 32 + smallX + (y * 32 + smallY) * 1920) * 4]
							= screen->packets[(y * 60 + x) / 212]->blocks[(y * 60 + x) % 212]
							.brightBlock[smallY / 8 * 4 + smallX / 8].data[smallX % 8 + (smallY % 8) * 8];
					}
				}
				for (int smallY = 0; smallY < 8; smallY++)
				{
					for (int smallX = 0; smallX < 8; smallX++)
					{
						rawPixelQuadData[(x * 32 / 4 + smallX + (y * 32 / 4 + smallY) * 1920) * 4 + 1]
							= screen->packets[(y * 60 + x) / 212]->blocks[(y * 60 + x) % 212]
							.colorBlockCb.data[smallX + smallY * 8];
						rawPixelQuadData[(x * 32 / 4 + smallX + (y * 32 / 4 + smallY) * 1920) * 4 + 2]
							= screen->packets[(y * 60 + x) / 212]->blocks[(y * 60 + x) % 212]
							.colorBlockCr.data[smallX + smallY * 8];
					}
				}
			}
		}
		std::cout << "bytes incoming: " << sumBytes << std::endl;
	}
public:
	~Decompressor()
	{
		free(rawPixelQuadData);
		delete screen;
	}
};

Decompressor* decompressor;