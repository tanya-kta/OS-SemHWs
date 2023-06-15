#include <fcntl.h>
#include <sys/mman.h>
#include <unistd.h>
#include <semaphore.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h> /* for memset() */
#include <arpa/inet.h>  /* for sockaddr_in and inet_addr() */
#include <sys/socket.h> /* for socket(), connect(), send(), and recv() */

#define MAX_LENGTH 1000

char *finish = "The End\n";
char *mem_name = "my memory to share";
int shmid;
char *message;
sem_t *rcvd;
sem_t *sndr;
int clnt_sock;
struct sockaddr_in clnt_addr;
unsigned int clnt_len = sizeof(clnt_addr); /* Length of client address data structure */


void dieWithError(char *msg) {
    perror(msg);
    exit(1);
}

int createTCPSocket(unsigned short port) {
    int sock;
    struct sockaddr_in echo_serv_addr; /* Local address */

    /* Create socket for incoming connections */
    if ((sock = socket(PF_INET, SOCK_STREAM, IPPROTO_TCP)) < 0) {
        dieWithError("socket() failed");
    }

    /* Construct local address structure */
    memset(&echo_serv_addr, 0, sizeof(echo_serv_addr));   /* Zero out structure */
    echo_serv_addr.sin_family = AF_INET;                /* Internet address family */
    echo_serv_addr.sin_addr.s_addr = htonl(INADDR_ANY); /* Any incoming interface */
    echo_serv_addr.sin_port = htons(port);              /* Local port */

    /* Bind to the local address */
    if (bind(sock, (struct sockaddr *) &echo_serv_addr, sizeof(echo_serv_addr)) < 0) {
        dieWithError("bind() failed");
    }

    /* Mark the socket so it will listen for incoming connections */
    if (listen(sock, 3) < 0) {
        dieWithError("listen() failed");
    }
    return sock;
}

void memShareInit() {
    if ((shmid = shm_open(mem_name, O_CREAT | O_RDWR, S_IRWXU)) == -1) {
        perror("shm_open");
        dieWithError("Memory opening error");
    } else {
        printf("Memory opened: name = %s, id = 0x%x\n", mem_name, shmid);
    }
    if (ftruncate(shmid, MAX_LENGTH + 2 * sizeof(sem_t)) == -1) {
        perror("ftruncate");
        dieWithError("Setting memory size error");
    } else {
        printf("Memory size set - %ld\n", MAX_LENGTH + 2 * sizeof(sem_t));
    }
    void *p = mmap(0, MAX_LENGTH + 2 * sizeof(sem_t), PROT_WRITE | PROT_READ, MAP_SHARED, shmid, 0);
    message = p;
    rcvd = p + MAX_LENGTH;
    sndr = rcvd + sizeof(sem_t);

    if (sem_init(rcvd, 1, 0) == -1) {
        dieWithError("sem_init(first) failed");
    }
    if (sem_init(sndr, 1, 0) == -1) {
        dieWithError("sem_init(second) failed");
    }
    printf("All semaphores are inited\n\n");
}

void receiver() {
    int bytes_rcvd;
    if ((bytes_rcvd = recv(clnt_sock, message, MAX_LENGTH - 1, 0)) < 0) {
        dieWithError("recv() failed");
    }

    for (;bytes_rcvd > 0;) {
        message[bytes_rcvd] = '\0';
        if (strcmp(finish, message) == 0) {
            break;
        }
        printf("1 %s", message);
        sem_post(sndr);
        sem_wait(rcvd);
        if ((bytes_rcvd = recv(clnt_sock, message, MAX_LENGTH - 1, 0)) < 0) {
            dieWithError("recv() failed");
        }
    }
    sem_post(sndr);
}

void sender() {
    for (;;) {
        sem_wait(sndr);
        int ln = strlen(message);
        printf("2 %s", message);
        if (send(clnt_sock, message, ln, 0) != ln) {
            dieWithError("send() failed");
        }
        if (strcmp(finish, message) == 0) {
            break;
        }
        sem_post(rcvd);
    }
}

int main(int argc, char *argv[]) {
    if (argc != 2) {
        fprintf(stderr, "Usage:  %s <Server Port>\n", argv[0]);
        exit(1);
    }

    int serv_sock = createTCPSocket(atoi(argv[1]));
    memShareInit();

    if (fork() < 0) {
        dieWithError("fork() failed");
    }

    if ((clnt_sock = accept(serv_sock, (struct sockaddr *) &clnt_addr, &clnt_len)) < 0) {
        dieWithError("accept() failed");
    }
    printf("Handling client %s\n", inet_ntoa(clnt_addr.sin_addr));

    if ((recv(clnt_sock, message, 1, 0)) < 0) {
        dieWithError("recv() failed");
    }
    if (message[0] == 's') {
        receiver();
    } else {
        sender();
    }

    close(clnt_sock);
    sem_destroy(rcvd);
    sem_destroy(sndr);

    printf("Closed all semaphores\n");

    if ((shmid = shm_open(mem_name, O_CREAT | O_RDWR, S_IRWXU)) == -1) {
        if (shm_unlink(mem_name) == -1) {
            perror("shm_unlink");
            dieWithError("Error getting pointer to shared memory");
        }
    }
    printf("Shared memory is unlinked\n");
}
