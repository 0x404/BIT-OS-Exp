/*
 * @author: 0x404
 * @Date: 2021-11-19 14:22:04
 * @LastEditTime: 2021-11-20 11:09:01
 * @Description: 
 */
#include <cstdio>
#include <cstring>
#include <iostream>
#include <cstdlib>
#include <unistd.h>
#include <sys/wait.h>
#include <sys/time.h>


void printTime(timeval &start, timeval &end)
{
    // 打印从start到end经历的时间
    // 先将时间转成微秒，逐级往上计算
    long long ls_us = (end.tv_sec * 1000000 + end.tv_usec) - (start.tv_sec * 1000000 + start.tv_usec);
    long long t = ls_us;
    long long us = t % 1000;
    t /= 1000;
    long long ms = t % 1000;
    t /= 1000;
    long long s = t % 60;
    t /= 60;
    long long m = t % 60;
    long long h = t / 60;
    std::cout << h << "小时" << m << "分" << s << "秒" << ms << "毫秒" << us << "微秒";
}

int main(int argc, char *argv[])
{
    if (argc == 2)
    {
        pid_t pid = fork();
        timeval start, end;
        gettimeofday(&start, NULL);
        if (pid > 0) // 父进程
        {
            std::cout << "[parent process] : create child process." << std::endl;
            wait(0);    // 等待子进程结束

            // 获取系统时间并输出
            gettimeofday(&end, NULL);
            std::cout << "[parent process] : runtime [";
            printTime(start, end);
            std::cout << "]" << std::endl;
            std::cout << "[parent process] : parent process finished." << std::endl;
        }
        else if (pid == 0) // 子进程
        {
            std::cout << "[child process] : child process start running." << std::endl;
            std::cout << "[child process] : child process call cmd " << argv[1] << std::endl;
            
            int res = execlp(argv[1], argv[1], NULL);   // 执行参数程序
            
            // 如果执行成功则不会执行下面的语句
            std::cout << "[child process] : cmd " << argv[1] << " not found" <<std::endl;   
            exit(0);
        }
    }
    else if (argc == 3)
    {
        pid_t pid = fork();
        timeval start, end;
        gettimeofday(&start, NULL);
        if (pid > 0) // 父进程
        {
            std::cout << "[parent process] : create child process." << std::endl;
            wait(0);    // 等待子进程结束

            // 获取系统时间并输出
            gettimeofday(&end, NULL);
            std::cout << "[parent process] : runtime [";
            printTime(start, end);
            std::cout << "]" << std::endl;
            std::cout << "[parent process] : parent process finished." << std::endl;
            exit(0);
        }
        else if (pid == 0) // 子进程
        {
            std::cout << "[child process] : child process start running." << std::endl;
            std::cout << "[child process] : child process call cmd " << argv[1] << std::endl;

            int res = execlp(argv[1], argv[1], argv[2], NULL);   // 执行参数程序，并传递时间参数

            // 如果执行成功则不会执行下面的语句
            std::cout << "[child process] : cmd " << argv[1] << " not found" <<std::endl;
            exit(0);
        }
    }
    else
    {
        std::cout << "use [mytime.exe cmd] to calculate runtime of cmd" << std::endl;
        std::cout << "use [mytime.exe cmd t] to run cmd1 t seconds" << std::endl;
    }
    return 0;
}
