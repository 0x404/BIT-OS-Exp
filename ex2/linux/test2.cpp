/*
 * @author: 0x404
 * @Date: 2021-11-19 14:22:04
 * @LastEditTime: 2021-11-20 11:09:28
 * @Description: 
 */

#include <cstdio>
#include <iostream>
#include <unistd.h>
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
		double limit = toDigit(argv[1]);
		usleep(limit * 1000000);    // 睡眠对应的时间
        std::cout <<"[child process] : " <<argv[1] << "s, finished" << std::endl;
		exit(0);

    }
    return 0;
}
