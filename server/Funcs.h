#pragma once
#include <vector>
#include <string>

std::vector <std::string> decodeData(char* INPUT, int SIZE)
{
    std::vector <std::string> data;
    std::string tmp = "";
    int pos = 0;
    data.push_back(tmp);
    for (int i = 0; i < SIZE; i++)
    {
        if (INPUT[i] == '\n')
        {
            data.push_back(tmp);
            pos++;
            continue;
        }
        else if (INPUT[i] == '\\')
        {
            data.push_back(tmp);
            break;
        }
        data[pos] += INPUT[i];
    }
    return data;
}

int stringToInt(std::string S)
{
    if (S.size() == 0)
        return 0;
    int n = 0;
    bool minus = false;
    if (S[0] == '-')
    {
        S = S.substr(1, S.size() - 1);
        minus = true;
    }
    for (int i = minus; i < S.size(); i++)
    {
        if (S[i] < '0' || S[i] > '9')
            break;
        n = n * 10 + int(S[i] - '0');
    }
    return n - 2 * n * minus;
}

std::string intToString(int N)
{
    if (N == 0)
        return "0";
    std::string s = "";
    bool minus = false;
    if (N < 0)
    {
        N *= -1;
        minus = true;
    }
    while (N > 0)
    {
        s = char('0' + N % 10) + s;
        N /= 10;
    }
    if (minus)
        s = "-" + s;
    return s;
}