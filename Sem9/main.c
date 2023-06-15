#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/ipc.h>
#include <sys/sem.h>
#include <signal.h>

key_t shm_key;
int semid = -1;
void (*prev)(int);



void handleCtrlC(int nsig){
    printf("Receive signal %d, CTRL-C pressed\n", nsig);

    for (int i = 0; semid != -1 && i < 2; ++i) {
        semctl(semid, i, IPC_RMID);
        printf("удален %d семафор\n", i);
    }
    printf("Удалены все семафоры или они не были созданы\n");
    exit(0);
}

int main(int argc, char **argv) {
    prev = signal(SIGINT, handleCtrlC);
    int fd[2];
    pipe(fd);

    shm_key = ftok("main.c", 0);
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

    pid_t ch;
    if ((ch = fork()) < 0) {
        printf("Проблемы с fork()\n");
        exit(1);
    }
    if (ch == 0) {
        signal(SIGINT, prev);
        struct sembuf parent_poster = { 0, 1, 0 };
        struct sembuf child_waiter = { 1, -1, 0 };

        char buffer[1010];

        for (int i = 0; i < 10; ++i) {
            semop(semid, &child_waiter, 1);
            size_t n_bytes = read(fd[0], buffer, 1000);
            buffer[n_bytes] = '\0';
            printf("Ребенок: получено сообщение\n%s\n", buffer);
            sprintf(buffer, "Сообщение номер %d\n", i);
            write(fd[1], buffer, strlen(buffer));
            semop(semid, &parent_poster, 1);
        }

    } else {
        struct sembuf child_poster = { 1, 1, 0 };
        struct sembuf parent_waiter = { 0, -1, 0 };

        char buffer[1010];

        for (int i = 0; i < 10; ++i) {
            sprintf(buffer, "Сообщение номер %d\n", i);
            write(fd[1], buffer, strlen(buffer));
            semop(semid, &child_poster, 1);
            semop(semid, &parent_waiter, 1);
            size_t n_bytes = read(fd[0], buffer, 1000);
            buffer[n_bytes] = '\0';
            printf("Родитель: получено сообщение\n%s\n", buffer);
        }
    }
    close(fd[0]);
    close(fd[1]);
}
