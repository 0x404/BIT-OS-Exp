/*
 * @author: 0x404
 * @Date: 2021-11-29 21:07:56
 * @LastEditTime: 2021-11-30 14:22:01
 * @Description: 
 */
#include <cstdio>
#include <cstring>
#include <iostream>
#include <algorithm>
#include <windows.h>
#include <ctime>
#include <cstdlib>
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
    srand(time(0)); // 随机数种子

    HANDLE mapFile = OpenFileMapping(FILE_MAP_ALL_ACCESS, 0, mappingName);  // 打开共享映射文件
    if (mapFile == NULL)
    {
        cout << "[consumer] : open file mapping failed." << endl;
        exit(0);
    }
    buffer_queue *shmPtr = static_cast<buffer_queue*>(MapViewOfFile(mapFile, FILE_MAP_ALL_ACCESS, 0, 0, sizeof(buffer_queue))); // 读取共享文件上的视图，并获取缓冲区指针
    if (shmPtr == NULL)
    {
        cout << "[consumer] : create map view failed." << endl;
        CloseHandle(mapFile);
        exit(0);
    }

    HANDLE mutex = OpenMutex(MUTEX_ALL_ACCESS, 1, mutexName);   // 打开互斥锁信号量
    if (mutex == NULL)
    {
        cout << "[consumer] : open mutex failed." << endl;
        CloseHandle(mapFile);
        return 0;
    }
    HANDLE empty = OpenSemaphore(SEMAPHORE_ALL_ACCESS, 1, emptyName);   // 打开empty信号量
    if (empty == NULL)
    {
        cout << "[consumer] : open empty semaphore failed." << endl;
        CloseHandle(mapFile);
        return 0;
    }
    HANDLE full = OpenSemaphore(SEMAPHORE_ALL_ACCESS, 1, fullName);     // 打开full信号量
    if (full == NULL)
    {
        cout << "[consumer] : open full semaphore failed." << endl;
        CloseHandle(mapFile);
        return 0;
    }

    for (int i = 1; i <= 4; ++i)    // 消费者进程进行4次消费
    {
        int sleepTime = rand() % 3000 + 1;
        Sleep(sleepTime);   // 随机睡眠0-3秒

        WaitForSingleObject(full, INFINITE);
        WaitForSingleObject(mutex, INFINITE);

        /*******************************************临界区开始**********************************************/

        int item = shmPtr->queue[shmPtr->head];
        shmPtr->head = (shmPtr->head + 1) % BUFFER_SIZE;
        if (shmPtr->head == shmPtr->tail) shmPtr->isEmpty = true;
        
        // 输出当前操作和当前缓冲区
        cout << "[consumer] : get " << item << " from buffer queue. ";
        int head = shmPtr->head;
        vector<int> items;
        while (head != shmPtr->tail)
        {
            items.push_back(shmPtr->queue[head]);
            head = (head + 1) % BUFFER_SIZE;
        }
        cout << "[buffer queue] : ";
        for (int i = 0; i < items.size(); ++i) cout << items[i] << " ";
        cout << endl;

        /*******************************************临界区结束**********************************************/

        ReleaseMutex(mutex);
        ReleaseSemaphore(empty, 1, NULL);
    }

    // 进程结束，关闭句柄进行资源释放
    UnmapViewOfFile(mapFile);
    CloseHandle(mapFile);
    CloseHandle(mutex);
    CloseHandle(empty);
    CloseHandle(full);
    exit(0);
}