#include "helpers.h"
#include <unordered_map>
void usage(char *file) {
    fprintf(stderr, "Usage: %s server_port\n", file);
    exit(0);
}

void close_all_sock(fd_set *fd, int fdmax) {
    for (int sock = 0; sock < fdmax; sock++) {
        if (FD_ISSET(sock, fd)) {
            close(sock);
        }
    }
}

bool close_server(fd_set read_fds, int fdmax) {
    char exit_message[10];
    memset(exit_message, 0, 10);
    fgets(exit_message, 6, stdin);
    if (!strcmp(exit_message, "quit\n")) {
        close_all_sock(&read_fds, fdmax);
        printf("Exiting..\n");
        return true;
    }
    printf("That is not a valid command -_- \n");
    return false;
}

void tokenize(string comm, string &s1, string &s2) {
    vector<string> results;

    boost::split(results, comm, [](char c) { return c == ' '; });
    s1 = results[0];
    if (results.size() > 1) {
        s2 = results[1];
    }
}

void send_to_all_players(vector<int> players, const char *msg) {
    int n = 0;
    for (long unsigned int k = 0; k < players.size(); k++) {
        n = send(players[k], msg, strlen(msg), 0);
        if (n < 0) {
            perror("Nu s-au trimis mesajele la playeri!");
        }
    }
}

typedef struct player {
    int vieti;
    int sockfd;
} player;

int main(int argc, char *argv[]) {

    int sockfd, newsockfd, portno;
    char buffer[BUFLEN];
    struct sockaddr_in serv_addr, cli_addr;
    int n, i, ret, first_player = -1, guess_number = -1;
    vector<int> players;
    string s;
    socklen_t clilen;

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
    // in multimea read_fds
    FD_SET(sockfd, &read_fds);
    FD_SET(0, &read_fds);
    fdmax = sockfd;

    int no_clients = 0;
    int exit_while = 1;

    while (exit_while) {
        tmp_fds = read_fds;

        ret = select(fdmax + 1, &tmp_fds, NULL, NULL, NULL);
        DIE(ret < 0, "select");

        if (FD_ISSET(0, &tmp_fds)) {
            char command[20];

            memset(command, 0, 20);
            fgets(command, 10, stdin);
            if (!strcmp(command, "quit\n")) {
                close_all_sock(&read_fds, fdmax);
                printf("Exiting..\n");

            } else if (!strcmp(command, "status\n")) {
                printf("There are %d clients connected!\n", no_clients);
            } else {
                printf("That is not a valid command -_- \n");
            }
            continue;
        }

        for (i = 0; i <= fdmax; i++) {
            if (FD_ISSET(i, &tmp_fds)) {
                if (i == sockfd) {
                    // a venit o cerere de conexiune pe socketul inactiv (cel cu listen),
                    // pe care serverul o accepta
                    clilen = sizeof(cli_addr);
                    newsockfd = accept(sockfd, (struct sockaddr *)&cli_addr, &clilen);
                    no_clients++;
                    if (first_player < 0) {
                        first_player = newsockfd;
                    }
                    DIE(newsockfd < 0, "accept");

                    // se adauga noul socket intors de accept() la multimea
                    // descriptorilor de citire
                    FD_SET(newsockfd, &read_fds);
                    if (newsockfd > fdmax) {
                        fdmax = newsockfd;
                    }
                    players.push_back(newsockfd);

                    if (guess_number > 0) {
                        ret = send(newsockfd, CHOOSED, strlen(CHOOSED), 0);
                        if (ret < 0) {
                            perror("send choosed");
                        }
                    }

                    printf("Noua conexiune de la %s, port %d, socket client %d\n",
                           inet_ntoa(cli_addr.sin_addr), ntohs(cli_addr.sin_port), newsockfd);
                } else {
                    // s-au primit date pe unul din socketii de client,
                    // asa ca serverul trebuie sa le receptioneze
                    memset(buffer, 0, BUFLEN);
                    n = recv(i, buffer, sizeof(buffer), 0);
                    DIE(n < 0, "recv");

                    if (n == 0) {
                        // conexiunea s-a inchis
                        if (i == first_player) {
                            printf("Primul jucator a iesit\n");
                            close_all_sock(&read_fds, fdmax);
                            exit_while = 0;
                            printf("Exiting..\n");
                            break;
                        }
                        for (long unsigned int k = 0; k < players.size(); k++) {
                            if (players[k] == i) {
                                players.erase(players.begin() + k);
                            }
                        }
                        close(i);

                        printf("Socket-ul client %d a inchis conexiunea\n", i);
                        no_clients--;

                        // se scoate din multimea de citire socketul inchis
                        FD_CLR(i, &read_fds);

                    } else {
                        s = buffer;

                        // daca este comanda set
                        if (!strncmp(buffer, SET, strlen(SET))) {
                            string s1, s2;
                            tokenize(s, s1, s2);
                            // daca este primul player, accept numarul
                            if (i == first_player) {
                                guess_number = atoi(s2.c_str());
                                send_to_all_players(players, CHOOSED);
                                // altfel inseamna ca nu are voie
                            } else {
                                send(i, ACCESS, strlen(ACCESS), 0);
                            }
                        // daca e alta comanda inseamna ca se primeste numarul
                        } else {
                            if (i == first_player) {
                                continue;
                            }
                            int number = atoi(buffer);
                            if (number == guess_number) {
                                printf("Numarul a fost ghicit\n");
                                send_to_all_players(players, RIGHT);
                                close_all_sock(&read_fds, fdmax);
                                exit_while = 0;
                                printf("Exiting..\n");
                                break;
                            } else {
                                // strcpy(buffer, "WRONG");
                                ret = send(i, WRONG, strlen(WRONG), 0);
                                DIE(ret < 0, "Send wrong");
                            }
                        }

                        printf("S-a primit de la clientul de pe socketul %d mesajul: %s\n", i, buffer);
                    }
                }
            }
        }
    }

    close(sockfd);

    return 0;
}
