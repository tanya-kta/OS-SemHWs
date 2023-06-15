#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <signal.h>

int main() {
    pid_t curr_pid = getpid();
    printf("PID процесса: %d\nВведите PID получателя: ", curr_pid);
    pid_t receiver;
    scanf("%d", &receiver);
    printf("Введите число: ");
    int number;
    scanf("%d", &number);
    int bits[32];
    for (int i = 0; i < 32; ++i) {
        bits[i] = number & 1;
        number >>= 1;
    }
    for (int i = 31; i >= 0; --i) {
        printf("%d ", bits[i]);
        if (bits[i] == 1) {
            kill(receiver, SIGUSR1);
        } else {
            kill(receiver, SIGUSR2);
        }
        usleep(10000);
    }
    printf("\n");
}
