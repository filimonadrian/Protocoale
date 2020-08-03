#include "helpers.h"
#include <arpa/inet.h>
#include <netinet/in.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>

void usage(char *file) {
    fprintf(stderr, "Usage: %s server_port\n", file);
    exit(0);
}

int find_game(int sockfd, player players[], int len) {
    for (int i = 0; i < len; i++) {
        if (players[i].player1 == sockfd || players[i].player2 == sockfd) {
            return i;
        }
    }
    return -1;
}

// typedef struct mesaj {
//     int sockfd;
//     int name;
// } mesaj;

int main(int argc, char *argv[]) {

    int sockfd, newsockfd, portno;
    char buffer[BUFLEN];
    struct sockaddr_in serv_addr, cli_addr;
    int n, i, ret, send_msg;
    socklen_t clilen;

    player p[25];

    fd_set read_fds; // multimea de citire folosita in select()
    fd_set tmp_fds;  // multime folosita temporar
    int fdmax;       // valoare maxima fd din multimea read_fds

    if (argc < 2) {
        usage(argv[0]);
    }

    // se goleste multimea de descriptori de citire (read_fds)
    // si multimea temporara (tmp_fds)
    FD_ZERO(&read_fds);
    FD_ZERO(&tmp_fds);

    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    DIE(sockfd < 0, "socket");

    portno = atoi(argv[1]);
    DIE(portno == 0, "atoi");

    memset((char *)&serv_addr, 0, sizeof(serv_addr));
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(portno);
    serv_addr.sin_addr.s_addr = INADDR_ANY;
    int enable = 1;

    if (setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &enable, sizeof(int)) == -1) {
        perror("setsocketopt");
        exit(1);
    }
    ret = bind(sockfd, (struct sockaddr *)&serv_addr, sizeof(struct sockaddr));
    DIE(ret < 0, "bind");

    ret = listen(sockfd, MAX_CLIENTS);
    DIE(ret < 0, "listen");

    // se adauga noul file descriptor (socketul pe care se asculta conexiuni)
    // sin multimea read_fds
    FD_SET(sockfd, &read_fds);
    fdmax = sockfd;

    int j = 0;
    int k = 0;

    while (1) {
        tmp_fds = read_fds;

        ret = select(fdmax + 1, &tmp_fds, NULL, NULL, NULL);
        DIE(ret < 0, "select");

        for (i = 0; i <= fdmax; i++) {
            if (FD_ISSET(i, &tmp_fds)) {
                if (i == sockfd) {
                    // a venit o cerere de conexiune pe socketul inactiv (cel cu listen),
                    // pe care serverul o accepta
                    clilen = sizeof(cli_addr);
                    newsockfd = accept(sockfd, (struct sockaddr *)&cli_addr, &clilen);

                    memset(buffer, 0, BUFLEN);
                    if (k == 0) {
                        p[j].player1 = newsockfd;
                        // printf("First player has id: %d\n", p[j].player1);
                        k++;
                    } else {
                        // send to first player x symbol
                        buffer[0] = 'x';
                        p[j].player2 = newsockfd;

                        send_msg = send(p[j].player1, buffer, strlen(buffer), 0);
                        if (send_msg < 0) {
                            perror("Can't send symbol to first player");
                        }

                        // send to second player 0 symbol
                        buffer[0] = 'z';
                        send_msg = send(p[j].player2, buffer, strlen(buffer), 0);
                        if (send_msg < 0) {
                            perror("Can't send symbol to first player");
                        }

                        k = 0;
                        j++;
                    }

                    DIE(newsockfd < 0, "accept");

                    // se adauga noul socket intors de accept() la multimea descriptorilor de citire
                    FD_SET(newsockfd, &read_fds);
                    if (newsockfd > fdmax) {
                        fdmax = newsockfd;
                    }

                    printf("Noua conexiune de la %s, port %d, socket client %d\n",
                           inet_ntoa(cli_addr.sin_addr), ntohs(cli_addr.sin_port), newsockfd);
                } else {
                    // s-au primit date pe unul din socketii de client,
                    // asa ca serverul trebuie sa le receptioneze
                    memset(buffer, 0, BUFLEN);
                    n = recv(i, buffer, sizeof(buffer), 0);
                    DIE(n < 0, "recv");

                    int index = find_game(i, p, j);
                    if (p[index].player1 == i) {
                        printf("trimit mesajul %d %d la %d\n",
                               buffer[0]-'0', buffer[2]-'0', p[index].player2);

                        send_msg = send(p[index].player2, buffer,
                                        strlen(buffer), 0);
                        if (send_msg < 0) {
                            perror("Can't send message to second player");
                        }
                    } else {
                        printf("trimit mesajul %d %d la %d\n",
                               buffer[0]-'0', buffer[2]-'0', p[index].player1);

                        send_msg = send(p[index].player1, buffer,
                                        strlen(buffer), 0);
                        if (send_msg < 0) {
                            perror("Can't send message to first player");
                        }
                    }

                    if (n == 0) {
                        // conexiunea s-a inchis
                        int index = find_game(i, p, j);
                        close(p[index].player1);
                        close(p[index].player2);
                        printf("Closed connection: %d %d\n",
                               p[index].player1, p[index].player2);

                        // se scoate din multimea de citire socketul inchis
                        FD_CLR(p[index].player1, &read_fds);
                        FD_CLR(p[index].player2, &read_fds);

                    } else {
                        // printf("S-a primit de la clientul de pe socketul %d mesajul: %s\n", i, buffer);
                    }
                }
            }
        }
    }

    close(sockfd);

    return 0;
}
