#include "helpers.h"

void usage(char *file) {
    fprintf(stderr, "Usage: %s server_port\n", file);
    exit(0);
}

void list_clients() {
}

void conn_to_client() {
}

int main(int argc, char *argv[]) {

    int sockfd, newsockfd, portno;
    char buffer[BUFLEN];
    struct sockaddr_in serv_addr, cli_addr;
    int n, i, ret, received_data = 0;
    vector<client> clients;
    socklen_t clilen;

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

    // se adauga noul file descriptor (socketul pe care se asculta conexiuni) in multimea read_fds
    FD_SET(sockfd, &read_fds);
    fdmax = sockfd;

    int j = 0;
    client c;

    while (1) {
        tmp_fds = read_fds;

        ret = select(fdmax + 1, &tmp_fds, NULL, NULL, NULL);
        if (ret < 0) {
            perror("Can't select new fd:");
        }

        for (i = 0; i <= fdmax; i++) {
            if (FD_ISSET(i, &tmp_fds)) {
                if (i == sockfd) {
                    // a venit o cerere de conexiune pe socketul inactiv (cel cu listen),
                    // pe care serverul o accepta

                    clilen = sizeof(cli_addr);
                    newsockfd = accept(sockfd, (struct sockaddr *)&cli_addr, &clilen);
                    DIE(newsockfd < 0, "accept");

                    c.port = ntohs(cli_addr.sin_port);
                    // c.ip = inet_ntoa(cli_addr.sin_addr);
                    strcpy(c.ip, inet_ntoa(cli_addr.sin_addr));
                    c.sockfd = newsockfd;

                    // se adauga noul socket intors de accept() la multimea descriptorilor de citire
                    FD_SET(newsockfd, &read_fds);
                    if (newsockfd > fdmax) {
                        fdmax = newsockfd;
                    }

                    /*clean buffer*/
                    memset(buffer, 0, BUFLEN);
                    received_data = recv(newsockfd, buffer, sizeof(buffer), 0);
                    if (received_data < 0) {
                        perror("Can't receive user id!");
                    }
                    /*unwrap the message */
                    /*The buffer contains just THE USER ID */
                    string user_name = buffer;

                    // c.name = user_name;
                    strcpy(c.name, buffer);
                    clients.push_back(c);

                    cout << c.name << c.ip <<":" << c.port << " " << c.sockfd << endl;

                    printf("Noua conexiune de la %s, port %d, socket client %d, name %s\n",
                           inet_ntoa(cli_addr.sin_addr), ntohs(cli_addr.sin_port), newsockfd, c.name);
                } else {
                    // s-au primit date pe unul din socketii de client,
                    // asa ca serverul trebuie sa le receptioneze
                    memset(buffer, 0, BUFLEN);
                    n = recv(i, buffer, sizeof(buffer), 0);
                    DIE(n < 0, "recv");

                    if (n == 0) {
                        // conexiunea s-a inchis
                        // se sterge clientul care a iesit din lista
                        for (int k = 0; k < clients.size(); k++) {
                            if (i == clients[k].sockfd == i) {
                                clients.erase(clients.begin() + k);
                            }
                        }
                        close(i);

                        // se scoate din multimea de citire socketul inchis
                        FD_CLR(i, &read_fds);

                        printf("Socket-ul client %d a inchis conexiunea\n", i);
                    } else {
                        if (!strcmp(buffer, "LIST")) {
                            printf("clientul %d vrea lista\n", i);
                            for (int k = 0; k < clients.size(); k++) {
                                memset(buffer, 0, BUFLEN);
                                memcpy(buffer, &clients[k], sizeof(client));
                                send(i, buffer, BUFLEN, 0);
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
