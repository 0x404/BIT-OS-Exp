/*
 * @author: 0x404
 * @Date: 2021-11-18 21:45:55
 * @LastEditTime: 2021-11-20 11:04:04
 * @Description: 
 */

#include <cstdio>
#include <iostream>
#include <windows.h>
#include <cstring>

using namespace std;

double toDigit(char s[])
{
    // 将字符串转成对应的double类型
    double ans = 0;
    int i = 0;
    for (; i < strlen(s); ++i)
    {
        if (s[i] == '.')
        {
            i++;
            break;
        }
        ans = ans * 10 + s[i] - '0';
    }
    double p = 0.1;
    for (; i < strlen(s); ++i, p *= 0.1)
    {
        ans += (s[i] - '0') * p;
    }
    return ans;
}

int main(int argc, char *argv[])
{
    if (argc == 2)
    {
        Sleep(toDigit(argv[1]) * 1000); // 睡眠对应的时间
        exit(0);
    }
    
    else cout << "[child process] : argc error." << endl;
    
    exit(0);
}
