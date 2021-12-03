#include <cstdio>
#include <cstring>
#include <iostream>
#include <algorithm>
#include <vector>
#include <cstdlib>
#include <unistd.h>
#include <sys/shm.h>
#include <sys/sem.h>
#include <sys/wait.h>

using namespace std;


const int BUFFER_SIZE = 4;  // 循环队列大小 = 缓冲区大小 + 1
const int MUTEX = 0, EMPTY = 1, FULL = 2;

struct buffer_queue // 定义缓冲区结构
{
    int queue[BUFFER_SIZE]; // 循环队列
    int head, tail;         // 循环队列头尾指针
    bool isEmpty;
};

union semun {
    int val;
    struct semid_ds *buf;
    unsigned short *array;
};

void P(int semId, int semNum)   // P操作
{
    sembuf semBuffer;
    semBuffer.sem_num = semNum;
    semBuffer.sem_flg = 0;
    semBuffer.sem_op = -1;
    semop(semId, &semBuffer, 1);
}

void V(int semId, int semNum)   // V操作
{
    sembuf semBuffer;
    semBuffer.sem_num = semNum;
    semBuffer.sem_flg = 0;
    semBuffer.sem_op = 1;
    semop(semId, &semBuffer, 1);
}

void producerFun(int shmId, int semId, int producerId)      // 生产者进程函数
{
    buffer_queue *shmPtr = static_cast<buffer_queue*>(shmat(shmId, 0, 0));  // 获取共享缓存区
    if (shmPtr == (void*)(-1))
    {
        cout << "[producer " << producerId << "] : shmat error." << endl;
        exit(-1);
    }

    int sleepTime = rand() % 2 + 1;
    sleep(sleepTime);   // 随机睡眠1-3秒

    P(semId, EMPTY);
    P(semId, MUTEX);

    /*******************************************临界区开始**********************************************/

    int item = rand() % 5 + 1;
    shmPtr->queue[shmPtr->tail] = item;     // 放入新的元素
    shmPtr->tail = (shmPtr->tail + 1) % BUFFER_SIZE;    // 尾指针向前移动
    if (shmPtr->isEmpty) shmPtr->isEmpty = false;   // 判断是否为空

    cout << "[producer " << producerId << "] : put " << item << " to buffer queue.   ";
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

    shmdt(shmPtr);

    /*******************************************临界区结束**********************************************/
    V(semId, MUTEX);
    V(semId, FULL);
}

void consumerFun(int shmId, int semId, int consumerId)      // 消费者进程函数
{
    buffer_queue *shmPtr = static_cast<buffer_queue*>(shmat(shmId, 0, 0));  // 获取共享缓存区
    if (shmPtr == (void*)(-1))
    {
        cout << "[consumer " << consumerId << "] : shmat error." << endl;
        exit(-1);
    }

    int sleepTime = rand() % 2 + 2;
    sleep(sleepTime);   // 随机睡眠1-3秒

    P(semId, FULL);
    P(semId, MUTEX);

    /*******************************************临界区开始**********************************************/

    int item = shmPtr->queue[shmPtr->head];     // 取出循环队列第一个元素
    shmPtr->head = (shmPtr->head + 1) % BUFFER_SIZE;    // 头指针向前移动
    if (shmPtr->head == shmPtr->tail && shmPtr->isEmpty) shmPtr->isEmpty = true;    // 判断是否为空

    cout << "[consumer " << consumerId << "] : get " << item << " from buffer queue. ";
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

    shmdt(shmPtr);

    /*******************************************临界区结束**********************************************/

    V(semId, MUTEX);
    V(semId, EMPTY);
}


int main()
{
    srand(time(0)); // 随机数种子
    int shmId = shmget(IPC_PRIVATE, sizeof(buffer_queue), 0600 | IPC_CREAT);    // 创建共享内存
    if (shmId == -1)
    {
        cout << "[main process] : shmget error." << endl;
        return 0;
    }
    buffer_queue *shmPtr = static_cast<buffer_queue*>(shmat(shmId, 0, 0));      // 将共享内存映射到进程的地址空间
    if (shmPtr == (void*)(-1))
    {
        cout << "[main process] : shmat error." << endl;
        return 0;
    }
    shmPtr->head = shmPtr->tail = 0;
    shmPtr->isEmpty = true;     // 初始化共享内存



    int semId = semget(IPC_PRIVATE, 3, 0600 | IPC_CREAT);   // 创建信号量集

    unsigned short initValArray[3] = {1, 3, 0};   // 三个信号量的初值 分别为MUTEX EMPTY FULL
    union semun semVal;
    semVal.array = initValArray;
    if (semctl(semId, 3, SETALL, semVal) == -1)             // 对信号集的三个信号量进行设置
    {
        cout << "[main process] : semctl error." << endl;
        return 0;
    }

    cout << "[main process] : start." << endl;


    for (int i = 1; i <= 2; ++i)
    {
        pid_t pid = fork();
        if (pid == 0)
        {
            for (int j = 1; j <= 6; ++j)
            {
                producerFun(shmId, semId, i);   // 生产者进程
            }
            exit(0);
        }
    }
    for (int i = 1; i <= 3; ++i)
    {
        pid_t pid = fork();
        if (pid == 0)
        {
            for (int j = 1; j <= 4; ++j)
            {
                consumerFun(shmId, semId, i);   // 消费者进程
            }
            exit(0);
        }
    }

    while (wait(NULL) != -1);
    cout << "[main process] : finished." << endl;
    return 0;
}
