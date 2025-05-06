#pragma once
#include <iostream>
#include <fstream>
#include <atomic>

std::atomic <bool> logEnabled = false;

bool doLog = true;

void resetLog()
{
	if (!doLog) return;
	std::ofstream fout("log.txt");
	fout << "=== RESTART HARMONY VIDEOSERVER ===\n";
	fout.close();
}

void logS(std::string S)
{
	if (!doLog) return;
	std::ofstream fout("log.txt", std::ios_base::app);
	fout << S << "\n";
	std::cout << S << "\n";
	fout.close();
}

void logSpecial(std::string S)
{
	if (!doLog || ! logEnabled) return;
	std::ofstream fout("log.txt", std::ios_base::app);
	fout << S << "\n";
	std::cout << S << "\n";
	fout.close();
}

void logN(int N)
{
	if (!doLog) return;
	std::ofstream fout("log.txt", std::ios_base::app);
	fout << N << "\n";
	std::cout << N << "\n";
	fout.close();
}