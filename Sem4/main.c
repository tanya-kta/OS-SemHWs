#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>

int main(int argc, char **argv) {
    if (argc != 3) {
        printf("Неправильные аргументы\n");
        exit(1);
    }
    int in_file = open(argv[1], O_RDONLY, S_IRWXU);
    int out_file = open(argv[2], O_CREAT | O_TRUNC | O_WRONLY, S_IRWXU);
    char buffer[1000];
    int size = read(in_file, buffer, sizeof(buffer));
    write(out_file, buffer, size);
    close(in_file);
    close(out_file);
}
