#include <sys/ipc.h>
#include <sys/shm.h>
#include <sys/sem.h>
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>

#define TCP_CONNECT_SHMKEY "tcp_connect_shmkey"
#define SHM_SIZE 1024

char* tcp_connect_shm_read() {
     // 生成一个唯一的键
    key_t key = ftok(TCP_CONNECT_SHMKEY, 65);

    // 获取共享内存段的ID
    int shmid = shmget(key, 1024, 0666 | IPC_CREAT);

    // 获取信号量集的ID
    int semid = semget(key, 1, 0666 | IPC_CREAT);

    // 等待信号量，信号量值减为0时继续
    struct sembuf sb = {0, -1, 0}; // sem_op = -1，等待信号量
    semop(semid, &sb, 1);

    // 将共享内存附加到进程地址空间
    char *str = (char*) shmat(shmid, (void*)0, 0);
    char* ret = (char*)malloc(strlen(str) + 1);
    strcpy(ret, str);

    // 读取共享内存中的数据
    printf("Data read from shared memory: %s\n", str);

    // 分离共享内存
    shmdt(str);

    // 删除共享内存段
    shmctl(shmid, IPC_RMID, NULL);

    // 删除信号量
    semctl(semid, 0, IPC_RMID);

    printf("Shared memory segment and semaphore marked for deletion\n");
    return ret;
}

void tcp_connect_shm_write() {
     // 生成一个唯一的键
    key_t key = ftok(TCP_CONNECT_SHMKEY, 65);

    // 创建共享内存段，大小为1024字节
    int shmid = shmget(key, SHM_SIZE, 0666 | IPC_CREAT);

    // 创建信号量集，包含1个信号量
    int semid = semget(key, 1, 0666 | IPC_CREAT);

    // 初始化信号量，设置为0
    semctl(semid, 0, SETVAL, 0);

    // 将共享内存附加到进程地址空间
    char *str = (char*) shmat(shmid, (void*)0, 0);

    // 检查共享内存是否有内容
    strcpy(str, "Hello from writer");
    printf("Data written to shared memory: %s\n", str);

    // 发送信号量，将其值设置为1
    struct sembuf sb = {0, 1, 0}; // sem_op = 1，增加信号量
    semop(semid, &sb, 1);

    // 分离共享内存
    shmdt(str);
}