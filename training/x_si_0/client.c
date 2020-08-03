#include "helpers.h"
#include <arpa/inet.h>
#include <netdb.h>
#include <netinet/in.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>

void usage(char *file) {
    fprintf(stderr, "Usage: %s server_address server_port\n", file);
    exit(0);
}

void print_table(int board[BOARD_SIZE][BOARD_SIZE]) {
    for (int i = 0; i < BOARD_SIZE; i++) {
        for (int j = 0; j < BOARD_SIZE; j++) {
            if (board[i][j] == x) {
                printf("x ");
            } else if (board[i][j] == o) {
                printf("0 ");
            } else {
                printf("_");
            }
        }
        printf("\n");
    }
    printf("\n");
}



// int inline_numbers(int a, int b, int c) {



// }   


// int verify_win(int board[BOARD_SIZE][BOARD_SIZE]) {
//     for (int i = 0; i < BOARD_SIZE; i++) {
//         for (int j = 0; j < BOARD_SIZE; j++) {

//         }
//     }

// }

int verify_finish(int board[BOARD_SIZE][BOARD_SIZE]) {
    for (int i = 0; i < BOARD_SIZE; i++) {
        for (int j = 0; j <BOARD_SIZE; j++) {
            if (board[i][j] == blank) {
                return 1;
            }
        }
    }
    return 0;
}

int main(int argc, char *argv[]) {
    int sockfd, n, ret;
    struct sockaddr_in serv_addr;
    char buffer[BUFLEN];
    char message[BUFLEN];
    int i = 0, j = 0, symbol = blank, in_game = 0, turn = 0;

    int board[BOARD_SIZE][BOARD_SIZE];
    for (int i = 0; i < BOARD_SIZE; ++i) {
        for (int j = 0; j < BOARD_SIZE; ++j) {
            board[i][j] = blank;
        }
    }

    if (argc < 3) {
        usage(argv[0]);
    }

    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    DIE(sockfd < 0, "socket");

    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(atoi(argv[2]));
    ret = inet_aton(argv[1], &serv_addr.sin_addr);
    DIE(ret == 0, "inet_aton");

    ret = connect(sockfd, (struct sockaddr *)&serv_addr, sizeof(serv_addr));
    DIE(ret < 0, "connect");

    fd_set read_fds; // multimea de citire folosita in select()
    fd_set tmp_fds;  // multime folosita temporar
    int fdmax;       // valoare maxima fd din multimea read_fds
    fdmax = sockfd;

    FD_ZERO(&read_fds);
    FD_ZERO(&tmp_fds);

    FD_SET(sockfd, &read_fds);
    FD_SET(0, &read_fds);
    tmp_fds = read_fds;

    while (1) {
        tmp_fds = read_fds;
        memset(message, 0, BUFLEN);

        ret = select(fdmax + 1, &tmp_fds, NULL, NULL, NULL);
        DIE(ret < 0, "select");

        if (FD_ISSET(0, &tmp_fds)) {

            // se citeste de la tastatura
            memset(buffer, 0, BUFLEN);
            fgets(buffer, BUFLEN - 1, stdin);

            if (!strcmp(buffer, "exit\n")) {
                break;
            }

            if (!strcmp(buffer, "symbol\n")) {
                if (symbol == blank) {
                    printf("Not in game\n");
                } else if (symbol == x) {
                    puts("x");
                } else if (symbol == o) {
                    puts("0\n");
                }
            }

            if (!in_game) {
                puts("Can't move! You don't have a partner\n");
                continue;
            }

            if (!turn) {
                printf("It's not your turn\n");
                continue;
            }

            i = buffer[0] - '0';
            j = buffer[2] - '0';

            if (i < 0 || i >= 3 || j < 0 || j >= 3) {
                printf("Wrong combination of indexes\n");
                continue;
            }
            printf("You have introduced %d %d\n", i, j);

            if (board[i][j] != blank) {
                printf("You can't do this move\n");
                continue;
            }

            board[i][j] = symbol;
            if(!verify_finish(board)) {
                printf("DRAW");
                break;
            }
            // se trimite mesaj la server cu cele 2 numere
            n = send(sockfd, buffer, strlen(buffer), 0);
            DIE(n < 0, "send");
            turn = 0;
            print_table(board);
        }

        if (FD_ISSET(sockfd, &tmp_fds)) {

            int rec_msg = recv(sockfd, message, sizeof(message), 0);
            if (rec_msg < 0) {
                perror("Can't receive message from server!\n");
            }
            if (rec_msg == 0) {
                break;
            }

            if (!in_game) {
                if (message[0] == 'x') {
                    symbol = x;
                    turn = 1;
                } else if (message[0] == 'z') {
                    symbol = o;
                    turn = 0;
                }
                in_game = 1;
                continue;
            }

            turn = 1;

            i = message[0] - '0';
            j = message[2] - '0';

            printf("rec: %s", message);
            board[i][j] = !symbol;
            print_table(board);
        }
    }

    close(sockfd);

    return 0;
}
