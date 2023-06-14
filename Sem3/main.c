#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

int main(int argc, char **argv) {
    if (argc != 2) {
        printf("Неправильные аргументы\n");
        exit(1);
    }
    int n = atoi(argv[1]);
    pid_t ch;
    if ((ch = fork()) < 0) {
        printf("Проблемы с fork()\n");
        exit(1);
    }
    if (ch == 0) {
        int64_t answer = 1;
        for (int i = 2; i <= n; ++i) {
            answer *= i;
        }
        printf("Факториал: %ld\n", answer);
    } else {
        int first = 0;
        int second = 1;
        for (int i = 3; i <= n; ++i) {
            int third = first + second;
            first = second;
            second = third;
        }
        printf("Число Фибоначчи: %d\n", second);
    }
}
