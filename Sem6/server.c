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
int *msg_p = NULL;  // адрес сообщения в разделяемой памяти

void serverHandleCtrlC(int nsig){
    printf("Receive signal %d, CTRL-C pressed\n", nsig);

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
        printf("Неверный запуск: %s <number of randoms>\n", argv[0]);
        exit(1);
    }
    int pros_num = atoi(argv[1]);
    signal(SIGINT, serverHandleCtrlC);

    shm_key = ftok(pathname_for_two, 0);
    if((shmid = shmget(shm_key, sizeof(int) * pros_num,
                        0666 | IPC_CREAT | IPC_EXCL)) < 0)  {
        if((shmid = shmget(shm_key, sizeof(int) * pros_num, 0)) < 0) {
            printf("Can\'t connect to shared memory\n");
            exit(-1);
        }
        msg_p = shmat(shmid, NULL, 0);
        printf("Connect to Shared Memory from server\n");
    } else {
        msg_p = shmat(shmid, NULL, 0);
        printf("New Shared Memory from server\n");
    }

    for (int i = 0; i < pros_num; ++i) {
        printf("%d ", msg_p[i]);
    }
    printf("\n");
    shmctl(shmid, IPC_RMID, NULL);
    return 0;
}
