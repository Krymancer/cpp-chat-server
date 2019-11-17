#include <arpa/inet.h>
#include <netinet/in.h>
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>

struct client_info {
    int sockno;
    char ip[INET_ADDRSTRLEN];
};

int clients[100];
int n = 0;

pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;
void sendtoall(char *message, int curr) {
    int i;
    pthread_mutex_lock(&mutex);
    for (i = 0; i < n; i++) {
        if (clients[i] != curr) {
            if (send(clients[i], message, strlen(message), 0) < 0) {
                perror("sending failure");
                continue;
            }
        }
    }
    pthread_mutex_unlock(&mutex);
}

void *recvmg(void *sock) {
    struct client_info cl = *((struct client_info *)sock);
    char msg[500];
    int len;
    int i;
    int j;

    while ((len = recv(cl.sockno, msg, 500, 0)) > 0) {
        msg[len] = '\0';
        sendtoall(msg, cl.sockno);
        memset(msg, '\0', sizeof(msg));
    }

    pthread_mutex_lock(&mutex);
    printf("%s Disconnected\n", cl.ip);

    for (i = 0; i < n; i++) {
        if (clients[i] == cl.sockno) {
            j = i;
            while (j < n - 1) {
                clients[j] = clients[j + 1];
                j++;
            }
        }
    }
    
    n--;

    pthread_mutex_unlock(&mutex);
}

int main(int argc, char *argv[]) {
    struct sockaddr_in my_addr, their_addr;
    int my_sock;
    int their_sock;
    socklen_t their_addr_size;
    int portno;
    pthread_t sendt, recvt;
    char msg[500];
    int len;
    struct client_info cl;
    char ip[INET_ADDRSTRLEN];

    if (argc > 2) {
        printf("Too many arguments");
        exit(1);
    }

    portno = atoi(argv[1]);
    my_sock = socket(AF_INET, SOCK_STREAM, 0);
    memset(my_addr.sin_zero, '\0', sizeof(my_addr.sin_zero));
    my_addr.sin_family = AF_INET;
    my_addr.sin_port = htons(portno);
    my_addr.sin_addr.s_addr = inet_addr("0.0.0.0");
    their_addr_size = sizeof(their_addr);

    if (bind(my_sock, (struct sockaddr *)&my_addr, sizeof(my_addr))) {
        perror("Binding unsuccessful");
        exit(1);
    }

    if (listen(my_sock, 5) != 0) {
        perror("Listening unsuccessful");
        exit(1);
    }

    while (true) {
        if ((their_sock = accept(my_sock, (struct sockaddr *)&their_addr, &their_addr_size)) < 0) {
            perror("Accept unsuccessful");
            exit(1);
        }

        pthread_mutex_lock(&mutex);
        inet_ntop(AF_INET, (struct sockaddr *)&their_addr, ip, INET_ADDRSTRLEN);
        printf("%s Connected\n", inet_ntoa(their_addr.sin_addr));
        cl.sockno = their_sock;
        strcpy(cl.ip, inet_ntoa(their_addr.sin_addr));

        clients[n] = their_sock;
        n++;

        pthread_create(&recvt, NULL, recvmg, &cl);
        pthread_mutex_unlock(&mutex);
    }

    return 0;
}