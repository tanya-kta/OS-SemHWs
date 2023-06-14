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
#include <time.h>

int shmid = -1;
char *mem_name = "my memory to share";
int *msg_p = NULL;  // адрес сообщения в разделяемой памяти

void sysErr(char *msg) {
    puts(msg);
    exit(1);
}

void clientHandleCtrlC(int nsig){
    printf("Receive signal %d, CTRL-C pressed\n", nsig);

    if (shmid != -1) {
        if (shm_unlink(mem_name) == -1) {
            perror("shm_unlink");
            sysErr("server: error getting pointer to shared memory");
        }
    }
    printf("Закрыта разделяемая память\n");
    exit(0);
}

int main(int argc, char **argv) {
    srand(time(NULL));
    if (argc != 2) {
        printf("Неверный запуск: ./%s <number of randoms>\n", argv[0]);
        exit(1);
    }
    int pros_num = atoi(argv[1]);
    signal(SIGINT, clientHandleCtrlC);

    if ((shmid = shm_open(mem_name, O_CREAT | O_RDWR, S_IRWXU)) == -1) {
        perror("shm_open");
        sysErr("server: object is already open");
    } else {
        printf("Object in parent is open: name = %s, id = 0x%x\n", mem_name, shmid);
    }
    // Задание размера объекта памяти
    if (ftruncate(shmid, sizeof(int) * pros_num) == -1) {
        perror("ftruncate");
        sysErr("server: memory sizing error");
    } else {
        printf("Memory size set and = %lu\n", sizeof(int) * pros_num);
    }
    // получить доступ к памяти
    msg_p = mmap(0, sizeof(int) * pros_num, PROT_WRITE | PROT_READ, MAP_SHARED, shmid, 0);


    for (int i = 0; i < pros_num; ++i) {
        msg_p[i] = rand() % 100;
        printf("%d ", msg_p[i]);
    }
    printf("\n");

    sleep(5);

    return 0;
}
