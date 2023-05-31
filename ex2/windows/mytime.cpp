/*
 * @author: 0x404
 * @Date: 2021-11-18 15:59:49
 * @LastEditTime: 2021-11-20 11:02:19
 * @Description: 14:16
 */

#include <cstdio>
#include <cstring>
#include <iostream>
#include <windows.h>

using namespace std;


void printTime(SYSTEMTIME &start, SYSTEMTIME &end)
{
    // 打印从start到end经历的时间
    // 先将时间转成毫秒，逐级往上计算
    long long start_ms = start.wMilliseconds + start.wSecond * 1000 + start.wMinute * 60 * 1000 + start.wHour * 60 * 60 * 1000;
    long long end_ms = end.wMilliseconds + end.wSecond * 1000 + end.wMinute * 60 * 1000 + end.wHour * 60 * 60 * 1000;

    long long t = end_ms - start_ms;
    long long ms = t % 1000;
    t /= 1000;
    long long s = t % 60;
    t /= 60;
    long long m = t % 60;
    long long h = t / 60;

    std::cout << h << "小时" << m << "分钟" << s << "秒" << ms << "毫秒";
}

int main(int argc, char *argv[])
{
    if (argc == 2)
    {
        STARTUPINFO si;
        PROCESS_INFORMATION pi;
        memset(&si, 0, sizeof(si)); // 初始化STARTUPINFO
        
        SYSTEMTIME start, end;
        
        GetSystemTime(&start);
        std::cout << "[parent process] : create child process." << std::endl;

        bool ok = CreateProcess(NULL, argv[1], NULL, NULL, FALSE, 0, NULL, NULL, &si, &pi);
        if (!ok)
        {
            std::cout << "[parent process] : create child process failed." << std::endl;
            return 0;
        }
        
        WaitForSingleObject(pi.hProcess, INFINITE);
        CloseHandle(pi.hProcess);   // 进程返回后释放资源
        CloseHandle(pi.hThread);    // 进程返回后释放资源


        GetSystemTime(&end);
        std::cout << "[parent process] : child process finished." << std::endl;
        std::cout << "[parent process] : child runtime [";
        printTime(start, end);
        std::cout << "]." << std::endl;
        std::cout << "[parent process] : parent process finished." << std::endl;
    }
    else if (argc == 3)
    {
        STARTUPINFO si;
        PROCESS_INFORMATION pi;
        memset(&si, 0, sizeof(si)); // 初始化STARTUPINFO
        
        SYSTEMTIME start, end;
        GetSystemTime(&start);
        std::cout << "[parent process] : create child process." << std::endl;

        TCHAR cmd[100]; // 拼接字符串，将两个参数拼接在一起
        int pos = 0;
        for (; pos < strlen(argv[1]); ++pos) cmd[pos] = argv[1][pos];
        cmd[pos++] = ' ';
        for (int i = 0; i < strlen(argv[2]); ++i, pos++) cmd[pos] = argv[2][i];
        cmd[pos] = '\0';

        bool ok = CreateProcess(NULL, cmd, NULL, NULL, FALSE, 0, NULL, NULL, &si, &pi);
        if (!ok)
        {
            std::cout << "[parent process] : create child process failed." << std::endl;
            return 0;
        }

        WaitForSingleObject(pi.hProcess, INFINITE);
        CloseHandle(pi.hProcess);   // 进程返回后释放资源
        CloseHandle(pi.hThread);    // 进程返回后释放资源


        GetSystemTime(&end);
        std::cout << "[parent process] : child process finished." << std::endl;
        std::cout << "[parent process] : child runtime [";
        printTime(start, end);
        std::cout << "]." << std::endl;
        std::cout << "[parent process] : parent process finished." << std::endl;
    }
    else
    {
        std::cout << "use [mytime.exe cmd1] to run cmd1" << std::endl;
        std::cout << "use [mytime.exe cmd1 t] to run cmd1 in t seconds" << std::endl;
    }
    return 0;
}

