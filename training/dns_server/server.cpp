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

void print_info(unordered_map<string, vector<string>> lookup,
                unordered_map<string, string> rev_lookup,
                unordered_map<string, vector<string>> mail_lookup) {

    cout << "Tabela Hostname : IP\n";
    for (auto entry : lookup) {
        cout << entry.first << " ";
        for (auto el : entry.second) {
            cout << el << " ";
        }
        cout << endl;
    }

    cout << "Tabela IP : HOSTNAME\n";
    for (auto entry : rev_lookup) {
        cout << entry.first << " " << entry.second << endl;
    }

    cout << "Tabela HOSTNAME : MAILSERVER\n";
    for (auto entry : mail_lookup) {
        cout << entry.first << " ";
        for (auto el : entry.second) {
            cout << el << " ";
        }
        cout << endl;
    }
}

void tokenize(string comm, string &s1, string &s2) {
    vector<string> results;

    boost::split(results, comm, [](char c) { return c == ' '; });
    s1 = results[1];
    if (results.size() > 2) {
        s2 = results[2];
    }
}

void tokenize2(char buffer[100], string &s1, string &s2) {
    char command[20], hostname[100], ip[100];
    sscanf(buffer, "%s %s %s", command, hostname, ip);
    s1 = hostname;
    s2 = ip;
}

typedef struct mesaj {
    int sockfd;
    int name;
} mesaj;

int main(int argc, char *argv[]) {

    int sockfd, newsockfd, portno;
    char buffer[BUFLEN];
    struct sockaddr_in serv_addr, cli_addr;
    int n, i, ret;
    string s;
    socklen_t clilen;
    unordered_map<string, vector<string>> lookup;
    unordered_map<string, string> rev_lookup;
    unordered_map<string, vector<string>> mail_lookup;

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

    int j = 0;
    int no_clients = 0;

    while (1) {
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
            } else if (!strcmp(command, "info\n")) {
                print_info(lookup, rev_lookup, mail_lookup);
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

                    j++;

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

                    if (n == 0) {
                        // conexiunea s-a inchis
                        printf("Socket-ul client %d a inchis conexiunea\n", i);
                        close(i);
                        no_clients--;

                        // se scoate din multimea de citire socketul inchis
                        FD_CLR(i, &read_fds);

                    } else {
                        s = buffer;
                        string s1, s2;
                        tokenize(s, s1, s2);

                        if (!strncmp(buffer, ADD_ADDRESS, strlen(ADD_ADDRESS))) {
                            if (lookup.find(s1) == lookup.end()) {
                                lookup.emplace(s1, vector<string>());
                                lookup[s1].push_back(s2);
                            } else {
                                lookup[s1].push_back(s2);
                            }
                        } else if (!strncmp(buffer, ADD_NAME, strlen(ADD_NAME))) {
                            if (rev_lookup.find(s1) == rev_lookup.end()) {
                                rev_lookup.emplace(s1, s2);
                            } else {
                                rev_lookup[s1] = s2;
                            }
                        } else if (!strncmp(buffer, ADD_MAIL, strlen(ADD_MAIL))) {
                            if (mail_lookup.find(s1) == mail_lookup.end()) {
                                mail_lookup.emplace(s1, vector<string>());
                                mail_lookup[s1].push_back(s2);
                            } else {
                                mail_lookup[s1].push_back(s2);
                            }

                        } else if (!strncmp(buffer, GET_ADDRESS, strlen(GET_ADDRESS))) {
                            if (lookup.find(s1) == lookup.end()) {
                                n = send(i, NO_ENTRY, strlen(NO_ENTRY), 0);
                                DIE(n < 0, "send get address");
                            } else {
                                memset(buffer, 0, BUFLEN);

                                for (int k = 0; k < lookup[s1].size(); k++) {
                                    strcat(buffer, lookup[s1][k].c_str());
                                    // cout << buffer << endl;
                                }

                                n = send(i, buffer, BUFLEN, 0);
                                DIE(n < 0, "send get address");
                            }
                        } else if (!strncmp(buffer, GET_NAME, strlen(GET_NAME))) {
                            if (rev_lookup.find(s1) == rev_lookup.end()) {
                                n = send(i, NO_ENTRY, strlen(NO_ENTRY), 0);
                                DIE(n < 0, "send get address");
                            } else {
                                memset(buffer, 0, BUFLEN);
                                strcpy(buffer, rev_lookup[s1].c_str());
                                n = send(i, buffer, BUFLEN, 0);
                                DIE(n < 0, "send get address");
                            }
                        } else if (!strncmp(buffer, GET_MAIL, strlen(GET_MAIL))) {
                            if (mail_lookup.find(s1) == mail_lookup.end()) {
                                n = send(i, NO_ENTRY, strlen(NO_ENTRY), 0);
                                DIE(n < 0, "send get mail");
                            } else {
                                for (int k = 0; k < mail_lookup[s1].size(); k++) {
                                    memset(buffer, 0, BUFLEN);
                                    strcat(buffer, mail_lookup[s1][k].c_str());
                                }
                                n = send(i, buffer, BUFLEN, 0);
                                DIE(n < 0, "send get mail");
                            }
                        } else {
                            cout << "Invalid command\n";
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
