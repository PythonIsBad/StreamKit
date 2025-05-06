#pragma once
#include <Windows.h>

const int fullXSize = 1920, fullYSize = 1080;
const int downsamplingSize = 4;

int screenSizeX, screenSizeY;

struct DCTBlock
{
	unsigned char loadPhase;
	unsigned char* data;
	DCTBlock()
	{
		loadPhase = 0;
		data = (unsigned char*)(malloc(64));
		for (int i = 0; i < 64; i++)
			data[i] = 0;
	}
	void copy(DCTBlock* target)
	{
		for (int i = 0; i < 64; i++)
		{
			data[i] = target->data[i];
			loadPhase = target->loadPhase;
		}
	}
	void refresh()
	{
		for (int i = 0; i < 64; i++)
			data[i] = 0;
		loadPhase = 0;
	}
	~DCTBlock()
	{
		free(data);
	}
};

struct ScreenBlock
{
	DCTBlock brightBlock[16];
	DCTBlock colorBlockCb;
	DCTBlock colorBlockCr;
	ScreenBlock() {}
	void copy(ScreenBlock* target)
	{
		for (int dctI = 0; dctI < 16; dctI++)
			brightBlock[dctI].copy(&(target->brightBlock[dctI]));
		colorBlockCb.copy(&(target->colorBlockCb));
		colorBlockCr.copy(&(target->colorBlockCr));
	}
	void refresh()
	{
		for (int dctI = 0; dctI < 16; dctI++)
			brightBlock[dctI].refresh();
		colorBlockCb.refresh();
		colorBlockCr.refresh();
	}
	~ScreenBlock() {}
};

struct StablePacket
{
	ScreenBlock blocks[212];
	char* encodedData;
	int encodedSize = 64872 + 10;
	StablePacket()
	{
		encodedData = (char*)(malloc(64872 + 10)); // 212 * 18 * 17
	}
	void copy(StablePacket* target)
	{
		for (int blockI = 0; blockI < 212; blockI++)
			blocks[blockI].copy(&(target->blocks[blockI]));
	}
	void refresh()
	{
		for (int blockI = 0; blockI < 212; blockI++)
			blocks[blockI].refresh();
	}
	~StablePacket()
	{
		free(encodedData);
	}
};

struct Screen
{
	StablePacket* packets[10];
	Screen()
	{
		for (int i = 0; i < 10; i++)
			packets[i] = new StablePacket();
	}
	void copy(Screen* target)
	{
		for (int packetI = 0; packetI < 10; packetI++)
			packets[packetI]->copy(target->packets[packetI]);
	}
	void refresh()
	{
		for (int packetI = 0; packetI < 10; packetI++)
			packets[packetI]->refresh();
	}
	~Screen()
	{
		for (int i = 0; i < 10; i++)
			delete packets[i];
	}
};