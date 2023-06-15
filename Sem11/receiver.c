#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h> /* for memset() */
#include <arpa/inet.h>  /* for sockaddr_in and inet_addr() */
#include <sys/socket.h> /* for socket(), connect(), send(), and recv() */

#define MAX_LENGTH 1000

void dieWithError(char *msg) {
    perror(msg);
    exit(1);
}

int main(int argc, char *argv[]) {
    if (argc != 3) {
        fprintf(stderr, "<Server IP> <Server Port>\n");
        exit(1);
    }

    int sock;
    if ((sock = socket(PF_INET, SOCK_STREAM, IPPROTO_TCP)) < 0) {
        dieWithError("socket() failed");
    }
    struct sockaddr_in serv_addr;
    memset(&serv_addr, 0, sizeof(serv_addr));    /* Zero out structure */
    serv_addr.sin_family = AF_INET;                  /* Internet address family */
    serv_addr.sin_addr.s_addr = inet_addr(argv[1]); /* Server IP address */
    serv_addr.sin_port = htons(atoi(argv[2]));
    if (connect(sock, (struct sockaddr *) &serv_addr, sizeof(serv_addr)) < 0) {
        dieWithError("connect() failed");
    }

    char message[MAX_LENGTH];
    message[0] = 'r';
    if (send(sock, message, 1, 0) != 1) {
        dieWithError("send() failed");
    }
    char *finish = "The End\n";
    int bytes_rcvd = 1;
    for (; bytes_rcvd > 0;) {
        if ((bytes_rcvd = recv(sock, message, MAX_LENGTH - 1, 0)) < 0) {
            dieWithError("recv() failed");
        }
        message[bytes_rcvd] = '\0';
        printf("Полученное сообщение:\n%s\n", message);
        if (strcmp(finish, message) == 0) {
            break;
        }
    }
    close(sock);
    exit(0);
}
