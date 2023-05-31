/*
 * @author: 0x404
 * @Date: 2021-11-29 20:12:07
 * @LastEditTime: 2021-11-30 10:59:32
 * @Description: 
 */

#include <cstdio>
#include <cstring>
#include <iostream>
#include <algorithm>
#include <windows.h>
#include <vector>

using namespace std;

const int BUFFER_SIZE = 4;  // 循环队列大小 = 缓冲区大小 + 1

TCHAR mappingName[] = TEXT("myFileMapping");    // 共享缓冲文件名
TCHAR mutexName[] = TEXT("myMutex");            // 互斥信号量名
TCHAR emptyName[] = TEXT("myEmpty");            // empty信号量名
TCHAR fullName[] = TEXT("myFull");              // full信号量名

struct buffer_queue // 缓冲区
{
    int queue[BUFFER_SIZE];     // 循环队列
    int head, tail;             // 头尾指针
    bool isEmpty;               // 队列是否为空
};


int main()
{
    HANDLE mapFile = CreateFileMapping(INVALID_HANDLE_VALUE, NULL, PAGE_READWRITE, 0, sizeof(buffer_queue), mappingName);   // 创建共享映射文件
    if (mapFile == NULL)
    {
        cout << "[main process] : create file mapping failed." << endl;
        return 0;
    }
    buffer_queue *shmPtr = static_cast<buffer_queue*>(MapViewOfFile(mapFile, FILE_MAP_ALL_ACCESS, 0, 0, sizeof(buffer_queue))); // 创建文件映射视图
    if (shmPtr == NULL)
    {
        cout << "[main process] : create map view failed." << endl;
        CloseHandle(mapFile);
        return 0;
    }
    // 初始化共享缓冲区
    shmPtr->head = shmPtr->tail = 0;
    shmPtr->isEmpty = true;

    HANDLE mutex = CreateMutex(NULL, 0, mutexName);     // 创建互斥信号量
    if (mutex == NULL)
    {
        cout << "[main porcess] : create mutex failed." << endl;
        CloseHandle(mapFile);
        return 0;
    }
    HANDLE empty = CreateSemaphore(NULL, 3, 3, emptyName);  // 创建empty信号量
    if (empty == NULL)
    {
        cout << "[main process] : create empty semaphore failed." << endl;
        CloseHandle(mapFile);
        return 0;
    }
    HANDLE full = CreateSemaphore(NULL, 0, 3, fullName);              // 创建full信号量
    if (full == NULL)
    {
        cout << "[main prcess] : create full semaphore failed." << endl;
        CloseHandle(mapFile);
        return 0;
    }

    PROCESS_INFORMATION pi_producer[2], pi_consumer[3];
    STARTUPINFO si_producer[2], si_consumer[3];
    for (int i = 0; i < 2; ++i)
    {
        // 初始化生产者进程的STARTUPINFO
        memset(&si_producer[i], 0, sizeof(si_producer[i]));
        si_producer[i].cb = sizeof(STARTUPINFO);
    }
    for (int i = 0; i < 3; ++i)
    {
        // 初始化消费者进程的STARTUPINFO
        memset(&si_consumer[i], 0, sizeof(si_consumer[i]));
        si_consumer[i].cb = sizeof(STARTUPINFO);
    }

    TCHAR producer[] = TEXT("producer.exe");
    TCHAR consumer[] = TEXT("consumer.exe");
    cout << "[main process] : start." << endl;

    for (int i = 0; i < 2; ++i)
    {
        // 创建生产者进程
        CreateProcess(NULL, producer, NULL, NULL, FALSE, 0, NULL, NULL, &si_producer[i], &pi_producer[i]);
    }
    for (int i = 0; i < 3; ++i)
    {
        // 创建消费者进程
        CreateProcess(NULL, consumer, NULL, NULL, FALSE, 0, NULL, NULL, &si_producer[i], &pi_producer[i]);
    }


    for (int i = 0; i < 2; ++i)
    {
        // 等待生产者进程返回，并关闭返回进程的句柄
        WaitForSingleObject(pi_producer[i].hProcess, INFINITE);
        CloseHandle(pi_producer[i].hThread);
        CloseHandle(pi_producer[i].hProcess);
    }
    for (int i = 0; i < 3; ++i)
    {
        // 等待消费者进程返回，并关闭返回进程的句柄
        WaitForSingleObject(pi_consumer[i].hProcess, INFINITE);
        CloseHandle(pi_consumer[i].hProcess);
        CloseHandle(pi_consumer[i].hThread);
    }
    
    UnmapViewOfFile(shmPtr);
    CloseHandle(mapFile);   // 关闭主进程共享内存句柄

    cout << "[main process] : finished." << endl;
    return 0;
}