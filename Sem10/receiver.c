#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <signal.h>

unsigned int received = 0;
int i = 0;

void handle0(int nsig){
    printf("Receive signal %d, SIGURS2\n", nsig);
    received <<= 1;
    i++;
}

void handle1(int nsig){
    printf("Receive signal %d, SIGURS1\n", nsig);
    received <<= 1;
    received += 1;
    i++;
}

int main() {
    (void)signal(SIGUSR2, handle0);
    (void)signal(SIGUSR1, handle1);

    pid_t curr_pid = getpid();
    printf("PID процесса: %d\nВведите PID отправилеля: ", curr_pid);
    pid_t sender;
    scanf("%d", &sender);
    while (i < 32);
    int *to_int = &received;
    printf("Полученное число: %d\n", *to_int);
}
