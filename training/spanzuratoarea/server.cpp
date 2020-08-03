#include "helpers.h"
#include <arpa/inet.h>
#include <iostream>
#include <map>
#include <netinet/in.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <string>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>
#include <unordered_map>

using namespace std;

void usage(char *file) {
    fprintf(stderr, "Usage: %s server_port\n", file);
    exit(0);
}

int main(int argc, char *argv[]) {
    int sockfd, newsockfd, portno;
    char buffer[BUFLEN];
    char ghiceste[30];
    int ghicit[30];

    struct sockaddr_in serv_addr, cli_addr;
    int n, i, ret, online = 0, first_client = -1;
    int cuvant_ales = -1;
    socklen_t clilen;

    // indicele este socketul, valoarea este numarul de vieti

    fd_set read_fds; // multimea de citire folosita in select()
    fd_set tmp_fds;  // multime folosita temporar
    int fdmax;       // valoare maxima fd din multimea read_fds

    if (argc < 2) {
        usage(argv[0]);
    }

    // se goleste multimea de descriptori de citire (read_fds) si multimea temporara (tmp_fds)
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

    ret = bind(sockfd, (struct sockaddr *)&serv_addr, sizeof(struct sockaddr));
    DIE(ret < 0, "bind");

    ret = listen(sockfd, MAX_CLIENTS);
    DIE(ret < 0, "listen");

    // se adauga noul file descriptor (socketul pe care se asculta conexiuni) in multimea read_fds
    FD_SET(sockfd, &read_fds);
    FD_SET(0, &read_fds);
    fdmax = sockfd;
    int stop_game = 1;
    while (stop_game) {
        tmp_fds = read_fds;
        ret = select(fdmax + 1, &tmp_fds, NULL, NULL, NULL);
        DIE(ret < 0, "select");

        for (i = 0; i <= fdmax; i++) {
            if (FD_ISSET(i, &tmp_fds)) {

                if (i == 0) {
                    memset(buffer, 0, BUFLEN);
                    fgets(buffer, 100, stdin);
                    if (!strcmp(buffer, "status\n")) {
                        printf("Sunt %d jucatori conectati\n", online);
                        continue;
                    } else if (strcmp(buffer, "quit\n")) {
                        stop_game = 0;
                        break;
                    }
                } else if (i == sockfd) {

                    clilen = sizeof(cli_addr);
                    newsockfd = accept(sockfd, (struct sockaddr *)&cli_addr, &clilen);
                    DIE(newsockfd < 0, "accept");

                    FD_SET(newsockfd, &read_fds);
                    if (newsockfd > fdmax) {
                        fdmax = newsockfd;
                    }

                    if (first_client < 0) {
                        first_client = newsockfd;
                    }

                    online++;
                    printf("Noua conexiune de la %s, port %d, socket client %d\n",
                           inet_ntoa(cli_addr.sin_addr), ntohs(cli_addr.sin_port), newsockfd);

                    // pentru noii clienti
                    if (cuvant_ales != 0) {
                    }

                } else {
                    // s-au primit date pe unul din socketii de client

                    memset(buffer, 0, BUFLEN);
                    n = recv(i, buffer, sizeof(buffer), 0);
                    DIE(n < 0, "recv");

                    if (n == 0) {
                        // conexiunea s-a inchis
                        online--;
                        close(i);
                        // se scoate din multimea de citire socketul inchis
                        FD_CLR(i, &read_fds);

                        // daca e chiar primul client, ies din joc
                        if (i == first_client) {
                            stop_game = 0;
                            break;
                        }
                    } else {

                        printf("Comanda primita: %s\n", buffer);

                        // daca e set si nu e primul client => eroare
                        if (strncmp(buffer, "set", 3) == 0) {
                            if (i != first_client) {

                                strcpy(buffer, "Nu poti seta cuvantul!");
                                n = send(i, buffer, strlen(buffer), 0);
                                DIE(n < 0, "Eroare set cuvant");
                                continue;

                            } else {
                                char aux[50], word[50];
                                sscanf(buffer, "%s %s", aux, word);
                                if (cuvant_ales > 0) {
                                    strcpy(buffer, "cuvantul a fost deja ales");
                                    n = send(i, buffer, strlen(buffer), 0);
                                    DIE(n < 0, "Eroare send");
                                    continue;
                                }

                                if (strlen(word) < 10 || strlen(word) > 25) {
                                    strcpy(buffer, "Nu are 10-25 litere");
                                    n = send(i, buffer, strlen(buffer), 0);
                                    DIE(n < 0, "Eroare numar litere");
                                    continue;
                                }

                                strcpy(ghiceste, word);
                                printf("Cuvantul ales este: %s\n", ghiceste);

                                cuvant_ales = 1;
                                for (int i = 0; i < strlen(ghiceste); i++) {
                                    ghicit[i] = 0;
                                }
                            }

                        } else if (strncmp(buffer, "guess", 5) == 0) {
                            if (i == first_client) {
                                strcpy(buffer, "Nu ai voie sa ghicesti!");
                                n = send(i, buffer, strlen(buffer), 0);
                                DIE(n < 0, "Eroare la send");
                                continue;
                            } else {
                                //guess + spatiu = 6 caractere
                                char litera[2];
                                litera[0] = buffer[6];
                                litera[1] = '\0';


                                if (cuvant_ales < 0) {
                                    strcpy(buffer, "Nu s-a ales un cuvant");
                                    n = send(i, buffer, strlen(buffer), 0);
                                    DIE(n < 0, "Eroare Send");
                                    continue;
                                }

                                char poz_litere_gasite[40];
                                strcpy(poz_litere_gasite, "Litera ");
                                strcat(poz_litere_gasite, litera);
                                strcat(poz_litere_gasite, " se gaseste pe: ");


                                int stare_litera = 0;

                                for (int j = 0; j < strlen(ghiceste); j++) {

                                    if (ghiceste[j] == litera[0]) {
                                        stare_litera = 1;
                                        ghicit[j] = 1;

                                        char poz[5];
                                        sprintf(poz, "%d ", j + 1);
                                        strcat(poz_litere_gasite, poz);
                                    }
                                }

                                if (stare_litera == 0) {
                                    memset(buffer, 0, BUFLEN);
                                    strcpy(buffer, "gresit");
                                    n = send(i, buffer, strlen(buffer), 0);
                                    DIE(n < 0, "eroare send");
                                    continue;

                                } else if (stare_litera == 1) {
                                    strcpy(buffer, poz_litere_gasite);
                                    for (int k = 4; k <= fdmax; k++) {
                                        if (FD_ISSET(k, &read_fds)) {
                                            n = send(k, buffer, strlen(buffer), 0);
                                            DIE(n < 0, "Mesaj toti userii");
                                        }
                                    }

                                    int aux = 1;
                                    for (int k = 0; k < strlen(ghiceste); k++) {
                                        if (ghicit[k] == 0) {
                                            aux = 0;
                                            break;
                                        }
                                    }

                                    if (aux == 1) {
                                        sprintf(buffer, "Felicitari! Cuvantul este %s\n", ghiceste);
                                        for (int k = 4; k <= fdmax; k++) {
                                            if (FD_ISSET(k, &read_fds)) {
                                                n = send(k, buffer, strlen(buffer), 0);
                                                DIE(n < 0, "Mesaj toti userii");
                                            }
                                        }
                                        stop_game = 0;
                                        break;
                                    }
                                }
                            }
                        }
                    }
                }
            }
        }
    }
    close(sockfd);
    return 0;
}