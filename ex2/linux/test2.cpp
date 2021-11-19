#include <cstdio>
#include <iostream>
#include <unistd.h>
#include <cstring>

using namespace std;

double toDigit(char s[])
{
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
		usleep(limit * 1000000);
        std::cout <<"[child process] : " <<argv[1] << "s, finished" << std::endl;
		exit(0);

    }
    return 0;
}
