#include <fcntl.h>
#include <stdio.h>
#include <sys/stat.h>
#include <stdlib.h>
#include <unistd.h>

int main(int argc, char **argv) {
    if (argc != 5) {
        printf("Параметры: ./%s <input file> <output file> <write channel> <read channel>", argv[0]);
        exit(1);
    }

    int input_file = open(argv[1], O_RDONLY, S_IRWXU);
    int output_file = open(argv[2], O_CREAT | O_TRUNC | O_WRONLY, S_IRWXU);

    umask(0);
    mknod(argv[3], S_IFIFO|0666, 0);
    int to_write = open(argv[3], O_WRONLY | O_CREAT);
    printf("Идет запись в канал %s\n", argv[3]);
    char buffer[1000];
    int size = read(input_file, buffer, sizeof(buffer));
    buffer[size] = '\0';
    //printf("%d %s %s\n", to_write, argv[3], buffer);
    write(to_write, buffer, size);


    umask(0);
    mknod(argv[4], S_IFIFO|0666, 0);
    int to_read = -1;
    while (to_read == -1) {
        to_read = open(argv[4], O_RDONLY);
        //printf("%d\n", to_read);
    }
    sleep(1);
    printf("Идет считывание из канала %s\n", argv[4]);
    size = read(to_read, buffer, sizeof(buffer));
    buffer[size] = '\0';
    //printf("%d %s %d %s\n", to_read, argv[4], size, buffer);
    write(output_file, buffer, size);

    close(input_file);
    close(output_file);
    close(to_read);
    close(to_write);
    unlink(argv[4]);
    return 0;
}