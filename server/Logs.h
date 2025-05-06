#pragma once
#include <iostream>
#include <fstream>
#include "Funcs.h"

int logID;
bool doLog = true;

void resetLog(int LOGID)
{
	logID = LOGID;
	if (!doLog) return;
	std::ofstream fout("log_client" + intToString(logID) + ".txt");
	fout << "=== RESTART HARMONY VIDEOCLIENT ===\n";
	fout.close();
}

void logS(std::string S)
{
	if (!doLog) return;
	std::ofstream fout("log_client" + intToString(logID) + ".txt", std::ios_base::app);
	fout << S << "\n";
	std::cout << S << "\n";
	fout.close();
}

void logN(int N)
{
	if (!doLog) return;
	std::ofstream fout("log_client" + intToString(logID) + ".txt", std::ios_base::app);
	fout << N << "\n";
	fout.close();
}