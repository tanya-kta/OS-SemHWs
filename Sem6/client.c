#include <stdio.h>
#include <sys/wait.h>
#include <sys/mman.h>
#include <semaphore.h>
#include <unistd.h>
#include <string.h>
#include <fcntl.h>
#include <sys/shm.h>
#include <stdlib.h>
#include <sys/sem.h>

int shmid = -1;
char pathname_for_two[]="client.c";
key_t shm_key;
int semid = -1;
int *msg_p = NULL;  // адрес сообщения в разделяемой памяти

void clientHandleCtrlC(int nsig){
    printf("Receive signal %d, CTRL-C pressed\n", nsig);

    for (int i = 0; semid != -1 && i < 2; ++i) {
        semctl(semid, i, IPC_RMID);
        printf("удален %d семафор\n", i);
    }
    printf("Удалены все семафоры или они не были созданы\n");
    if (msg_p != NULL) {
        shmdt(msg_p);
    }
    if (shmid != -1) {
        shmctl(shmid, IPC_RMID, NULL);
    }
    printf("Закрыта разделяемая память\n");
    exit(0);
}

int main(int argc, char **argv) {
    if (argc != 2) {
        printf("Неверный запуск: ./%s <number of randoms>\n", argv[0]);
        exit(1);
    }
    int pros_num = atoi(argv[1]);
    signal(SIGINT, clientHandleCtrlC);

    shm_key = ftok(pathname_for_two, 0);
    if ((semid = semget(shm_key, 2, 0666 | IPC_CREAT | IPC_EXCL)) < 0){
        if ((semid = semget(shm_key, 2, 0)) < 0) {
            printf("Can\'t connect to semaphor\n");
            exit(-1);
        }
        printf("Connect to Semaphor\n");
    } else {
        for (int i = 0; i < 2; ++i) {
            semctl(semid, i, SETVAL, 0);
        }
        printf("All new semaphores initialized\n");
    }
    if((shmid = shmget(shm_key, sizeof(int) * pros_num,
                        0666 | IPC_CREAT | IPC_EXCL)) < 0)  {
        if((shmid = shmget(shm_key, sizeof(int) * pros_num, 0)) < 0) {
            printf("Can\'t connect to shared memory\n");
            exit(-1);
        }
        msg_p = shmat(shmid, NULL, 0);
        printf("Connect to Shared Memory from client\n");
    } else {
        msg_p = shmat(shmid, NULL, 0);
        printf("New Shared Memory from client\n");
    }

    for (int i = 0; i < pros_num; ++i) {
        msg_p[i] = rand() % 100;
        printf("%d ", msg_p[i]);
    }
    printf("\n");
    struct sembuf to_server_poster = { 0, 1, 0 };
    struct sembuf client_waiter = { 1, -1, 0 };
    semop(semid, &to_server_poster, 1);
    semop(semid, &client_waiter, 1);

    for (int i = 0; semid != -1 && i < 2; ++i) {
        semctl(semid, i, IPC_RMID);
        printf("удален %d семафор\n", i);
    }
    return 0;
}
