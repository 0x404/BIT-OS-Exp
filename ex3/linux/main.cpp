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


const int BUFFER_SIZE = 3;
const int MUTEX = 0, EMPTY = 1, FULL = 2;

struct buffer_queue
{
    int queue[BUFFER_SIZE];
    int head, tail;
    bool isEmpty;
};

union semun {
    int val;
    struct semid_ds *buf;
    unsigned short *array;
};

void P(int semId, int semNum)
{
    sembuf semBuffer;
    semBuffer.sem_num = semNum;
    semBuffer.sem_flg = 0;
    semBuffer.sem_op = -1;
    semop(semId, &semBuffer, 1);
}

void V(int semId, int semNum)
{
    sembuf semBuffer;
    semBuffer.sem_num = semNum;
    semBuffer.sem_flg = 0;
    semBuffer.sem_op = 1;
    semop(semId, &semBuffer, 1);
}

void producerFun(int shmId, int semId, int producerId)
{
    buffer_queue *shmPtr = static_cast<buffer_queue*>(shmat(shmId, 0, 0));
    if (shmPtr == (void*)(-1))
    {
        cout << "[producer " << producerId << "] : shmat error." << endl;
        exit(-1);
    }
    int sleepTime = rand() % 5 + 1;
    // cout << "[producer " << producerId << "] : ready to produce. (sleep " << sleepTime << "s)." << endl;
    sleep(rand() % 5 + 1);

    P(semId, EMPTY);
    P(semId, MUTEX);

    int item = rand() % 5 + 1;
    shmPtr->queue[shmPtr->tail] = item;
    shmPtr->tail = (shmPtr->tail + 1) % BUFFER_SIZE;
    if (shmPtr->isEmpty) shmPtr->isEmpty = false;
    cout << "[producer " << producerId << "] : put " << item << " to buffer queue." << endl;
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
    V(semId, MUTEX);
    V(semId, FULL);    
}

void consumerFun(int shmId, int semId, int consumerId)
{
    buffer_queue *shmPtr = static_cast<buffer_queue*>(shmat(shmId, 0, 0));
    if (shmPtr == (void*)(-1))
    {
        cout << "[consumer " << consumerId << "] : shmat error." << endl;
        exit(-1);
    }
    int sleepTime = rand() % 5 + 1;
    // cout << "[comsumer " << consumerId << "] : ready to comsume. (sleep " << sleepTime << "s)." << endl;
    sleep(rand() % 5 + 1);

    P(semId, FULL);
    P(semId, MUTEX);

    int item = shmPtr->queue[shmPtr->head];
    shmPtr->head = (shmPtr->head + 1) % BUFFER_SIZE;
    if (shmPtr->head == shmPtr->tail && shmPtr->isEmpty) shmPtr->isEmpty = true;

    cout << "[consumer " << consumerId << "] : get " << item << " from buffer queue." << endl;
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
    V(semId, MUTEX);
    V(semId, EMPTY);
}


int main()
{
    srand(time(0));
    int shmId = shmget(IPC_PRIVATE, sizeof(buffer_queue), 0600 | IPC_CREAT);
    if (shmId == -1)
    {
        cout << "[main process] : shmget error." << endl;
        return 0;
    }
    buffer_queue *shmPtr = static_cast<buffer_queue*>(shmat(shmId, 0, 0));
    if (shmPtr == (void*)(-1))
    {
        cout << "[main process] : shmat error." << endl;
        return 0;
    }
    shmPtr->head = shmPtr->tail = 0;
    shmPtr->isEmpty = true;

    int semId = semget(IPC_PRIVATE, 3, 0600 | IPC_CREAT);

    unsigned short initValArray[3] = {1, BUFFER_SIZE, 0};
    union semun semVal;
    semVal.array = initValArray;
    if (semctl(semId, 3, SETALL, semVal) == -1)
    {
        cout << "[main process] : semctl error." << endl;
        return 0;
    }


    for (int i = 1; i <= 2; ++i)
    {
        pid_t pid = fork();
        if (pid == 0)
        {
            for (int j = 1; j <= 6; ++j)
            {
                producerFun(shmId, semId, i);
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
                consumerFun(shmId, semId, i);
            }
            exit(0);
        }
    }

    while (wait(NULL) != -1);
    cout << "[main process] : finished." << endl;
    return 0;
}
