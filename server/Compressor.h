#pragma once

#include <iostream>
#include "SFML/Graphics.hpp"
#include <Windows.h>
#include "ScreenBase.h"
#include "UserID.h"
#include <atomic>

class Compressor
{
private:
	int smallXSize = 1920, smallYSize = 1080;
	int shaderXSize = 1920, shaderYSize = 1088;
	unsigned char* screenPixelData;
	unsigned char* rawPixelData;
	unsigned char* pixelData;
	int ScreenshotSize;
	HDC ScreenDC;
	HDC MemoryDC;
	BITMAPINFO BMI;
	HBITMAP hBitmap;
	int count, emptyCounter, encodedBytes;
	DCTBlock* tmpDCTBlock;
	StablePacket* tmpPacket;
	sf::Texture texture, rawTexture;
	sf::RenderTexture renderTexture;
	sf::Sprite sprite, rawSprite;
	sf::Shader shader;
	sf::Image image;
	bool tmpBool;
public:
	Screen* oldScreen;
	Screen* newScreen;
	int sumBytes;
	std::atomic <bool> resetStream;
public:
	Compressor()
	{
		ScreenDC = GetDC(0);
		MemoryDC = CreateCompatibleDC(ScreenDC);
		//fullXSize = GetSystemMetrics(SM_CXSCREEN);
		//fullYSize = GetSystemMetrics(SM_CYSCREEN);
		BMI.bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
		BMI.bmiHeader.biWidth = fullXSize;
		BMI.bmiHeader.biHeight = -fullYSize;
		BMI.bmiHeader.biSizeImage = fullXSize * fullYSize * 3;
		BMI.bmiHeader.biCompression = BI_RGB;
		BMI.bmiHeader.biBitCount = 24;
		BMI.bmiHeader.biPlanes = 1;
		ScreenshotSize = BMI.bmiHeader.biSizeImage;
		screenPixelData = (unsigned char*)(malloc(shaderXSize * shaderYSize * 3));
		rawPixelData = (unsigned char*)(malloc(shaderXSize * shaderYSize * 4));
		pixelData = (unsigned char*)(malloc(shaderXSize * shaderYSize * 3));
		hBitmap = CreateDIBSection(ScreenDC, &BMI, DIB_RGB_COLORS, (void**)&screenPixelData, 0, 0);
		SelectObject(MemoryDC, hBitmap);
		for (int i = 0; i < smallXSize * smallYSize; i++)
			rawPixelData[i * 4 + 3] = 255;
		for (int i = smallXSize * smallYSize * 4; i < shaderXSize * shaderYSize * 4; i++)
			rawPixelData[i] = 0;
		texture.create(shaderXSize, shaderYSize);
		rawTexture.create(shaderXSize, shaderYSize);
		renderTexture.create(shaderXSize, shaderYSize);
		image.create(shaderXSize, shaderYSize);
		shader.loadFromFile("shaders/stable_compress_shader.frag", sf::Shader::Fragment);
		shader.setUniform("screen_resolution", sf::Vector2f(shaderXSize, shaderYSize));
		sprite.setTexture(texture);
		rawSprite.setTexture(rawTexture);
		oldScreen = new Screen();
		newScreen = new Screen();
		resetStream = false;
	}
	void runCompression()
	{
		BitBlt(MemoryDC, 0, 0, fullXSize, fullYSize, ScreenDC, 0, 0, SRCCOPY);
		for (int i = 0; i < smallXSize * smallYSize; i++)
		{
			rawPixelData[i * 4] = screenPixelData[i * 3];
			rawPixelData[i * 4 + 1] = screenPixelData[i * 3 + 1];
			rawPixelData[i * 4 + 2] = screenPixelData[i * 3 + 2];
		}
		rawTexture.update(rawPixelData);
		renderTexture.draw(rawSprite, &shader);
		renderTexture.display();
		texture.update(renderTexture.getTexture());
		image = renderTexture.getTexture().copyToImage();
		const unsigned char* tmpData = image.getPixelsPtr();
		for (int i = 0; i < smallXSize * smallYSize; i++)
		{
			pixelData[i * 3] = tmpData[i * 4] + 128;
			pixelData[i * 3 + 1] = tmpData[i * 4 + 1] + 128;
			pixelData[i * 3 + 2] = tmpData[i * 4 + 2] + 128;
		}
		countCompress();
		oldScreen->copy(newScreen);
	}
private:
	void countCompress()
	{
		tmpBool = resetStream;
		resetStream = false;
		for (int y = 0; y < 34; y++)
		{
			for (int x = 0; x < 60; x++)
			{
				for (int smallY = 0; smallY < 4; smallY++)
				{
					for (int smallX = 0; smallX < 4; smallX++)
					{
						newScreen->packets[(y * 60 + x) / 212]->blocks[(y * 60 + x) % 212]
							.brightBlock[smallY * 4 + smallX].loadPhase++;
					}
				}
				newScreen->packets[(y * 60 + x) / 212]->blocks[(y * 60 + x) % 212]
					.colorBlockCb.loadPhase++;
				newScreen->packets[(y * 60 + x) / 212]->blocks[(y * 60 + x) % 212]
					.colorBlockCr.loadPhase++;
				for (int smallY = 0; smallY < 32; smallY++)
				{
					for (int smallX = 0; smallX < 32; smallX++)
					{
						newScreen->packets[(y * 60 + x) / 212]->blocks[(y * 60 + x) % 212]
							.brightBlock[smallY / 8 * 4 + smallX / 8].data[smallX % 8 + (smallY % 8) * 8]
							= pixelData[(x * 32 + smallX + (y * 32 + smallY) * 1920) * 3];
						if (tmpBool || newScreen->packets[(y * 60 + x) / 212]->blocks[(y * 60 + x) % 212]
							.brightBlock[smallY / 8 * 4 + smallX / 8].data[smallX % 8 + (smallY % 8) * 8]
							!= oldScreen->packets[(y * 60 + x) / 212]->blocks[(y * 60 + x) % 212]
							.brightBlock[smallY / 8 * 4 + smallX / 8].data[smallX % 8 + (smallY % 8) * 8])
						{
							newScreen->packets[(y * 60 + x) / 212]->blocks[(y * 60 + x) % 212]
								.brightBlock[smallY / 8 * 4 + smallX / 8].loadPhase = 0;
						}
					}
				}
				for (int smallY = 0; smallY < 8; smallY++)
				{
					for (int smallX = 0; smallX < 8; smallX++)
					{
						newScreen->packets[(y * 60 + x) / 212]->blocks[(y * 60 + x) % 212]
							.colorBlockCb.data[smallX + smallY * 8]
							= pixelData[(x * 32 / 4 + smallX + (y * 32 / 4 + smallY) * 1920) * 3 + 1];
						if (tmpBool || newScreen->packets[(y * 60 + x) / 212]->blocks[(y * 60 + x) % 212]
							.colorBlockCb.data[smallX + smallY * 8]
							!= oldScreen->packets[(y * 60 + x) / 212]->blocks[(y * 60 + x) % 212]
							.colorBlockCb.data[smallX + smallY * 8])
						{
							newScreen->packets[(y * 60 + x) / 212]->blocks[(y * 60 + x) % 212]
								.colorBlockCb.loadPhase = 0;
						}
						newScreen->packets[(y * 60 + x) / 212]->blocks[(y * 60 + x) % 212]
							.colorBlockCr.data[smallX + smallY * 8]
							= pixelData[(x * 32 / 4 + smallX + (y * 32 / 4 + smallY) * 1920) * 3 + 2];
						if (tmpBool || newScreen->packets[(y * 60 + x) / 212]->blocks[(y * 60 + x) % 212]
							.colorBlockCr.data[smallX + smallY * 8]
							!= oldScreen->packets[(y * 60 + x) / 212]->blocks[(y * 60 + x) % 212]
							.colorBlockCr.data[smallX + smallY * 8])
						{
							newScreen->packets[(y * 60 + x) / 212]->blocks[(y * 60 + x) % 212]
								.colorBlockCr.loadPhase = 0;
						}
					}
				}
			}
		}

		// COMPRESS
		/*
		2040 blocks need
		306 bytes for one block
		64 872 KB = 212 blocks for packet
		10 packets
		*/

		sumBytes = 0;
		for (int packetI = 0; packetI < 10; packetI++)
		{
			tmpPacket = newScreen->packets[packetI];
			tmpPacket->encodedData[0] = 'S';
			tmpPacket->encodedData[1] = 'T';
			tmpPacket->encodedData[2] = 'R';
			tmpPacket->encodedData[3] = 'E';
			tmpPacket->encodedData[4] = 'A';
			tmpPacket->encodedData[5] = 'M';
			tmpPacket->encodedData[6] = userID;
			tmpPacket->encodedData[7] = roomID;
			tmpPacket->encodedData[8] = packetI;
			encodedBytes = 9;
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
					if (tmpDCTBlock->loadPhase < 4)
					{
						count = 0;
						emptyCounter = (3 - tmpDCTBlock->loadPhase) * 16;
						for (int i = 7; i >= 0; i--)
						{
							for (int j = 7 - i; j >= 0; j--)
							{
								if (emptyCounter > 0)
									emptyCounter--;
								else if (tmpDCTBlock->data[7 - j + 8 * (i + j)] == 128)
									count++;
								else
									goto exit1;
							}
						}
						for (int i = 6; i >= 0; i--)
						{
							for (int j = i; j >= 0; j--)
							{
								if (emptyCounter > 0)
									emptyCounter--;
								else if (tmpDCTBlock->data[i - j + 8 * j] == 128)
									count++;
								else
									goto exit1;
							}
						}
					exit1:
						if (count > 16)
							count = 16;
						emptyCounter = tmpDCTBlock->loadPhase * 16;
						count = 16 - count;
						tmpPacket->encodedData[encodedBytes] = count * 4 + int(tmpDCTBlock->loadPhase);
						encodedBytes++;
						for (int i = 0; i < 8; i++)
						{
							for (int j = 0; j <= i; j++)
							{
								if (emptyCounter > 0)
									emptyCounter--;
								else if (count > 0)
								{
									tmpPacket->encodedData[encodedBytes] = tmpDCTBlock->data[i - j + 8 * j] - 128;
									encodedBytes++;
									count--;
								}
								else
									goto exit2;
							}
						}
						for (int i = 1; i < 8; i++)
						{
							for (int j = 0; j < 8 - i; j++)
							{
								if (emptyCounter > 0)
									emptyCounter--;
								else if (count > 0)
								{
									tmpPacket->encodedData[encodedBytes] = tmpDCTBlock->data[7 - j + 8 * (i + j)] - 128;
									encodedBytes++;
									count--;
								}
								else
									goto exit2;
							}
						}
					exit2:
						continue;
					}
					else
					{
						tmpDCTBlock->loadPhase = 4;
						tmpPacket->encodedData[encodedBytes] = 127;
						encodedBytes++;
					}
				}
				tmpPacket->encodedSize = encodedBytes;
			}
			//std::cout << "Packet " << packetI << ": " << encodedBytes << "\n";
			sumBytes += encodedBytes;
		}
		std::cout << "Bytes sended: " << sumBytes << "\n";
	}
public:
	~Compressor()
	{
		free(rawPixelData);
		free(pixelData);
		delete oldScreen;
		delete newScreen;
	}
};

Compressor* compressor;