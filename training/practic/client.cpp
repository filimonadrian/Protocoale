#include "helpers.h"

void usage(char *file) {
    fprintf(stderr, "Usage: %s server_address server_port\n", file);
    exit(0);
}

// returns a socket
int open_connection(char *host_ip, int portno, int ip_type, int socket_type, int flag) {
    int ret = 0;
    struct sockaddr_in serv_addr;
    int sockfd = socket(ip_type, socket_type, flag);
    if (sockfd < 0)
        perror("ERROR opening socket");

    memset(&serv_addr, 0, sizeof(serv_addr));
    serv_addr.sin_family = ip_type;
    serv_addr.sin_port = htons(portno);
    ret = inet_aton(host_ip, &serv_addr.sin_addr);
    if (ret < 0) {
        perror("inet aton");
    }

    /* connect the socket */
    if (connect(sockfd, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0)
        perror("ERROR connecting");
    // exit(0);

    return sockfd;
}

// send a long message to server
void send_to_server(int sockfd, char *message) {
    int bytes, sent = 0;
    int total = strlen(message);

    do {
        bytes = write(sockfd, message + sent, total - sent);
        if (bytes < 0) {
            perror("ERROR writing message to socket");
        }

        if (bytes == 0) {
            break;
        }

        sent += bytes;
    } while (sent < total);
}

void tokenize(string comm, string &s1, string &s2) {
    vector<string> results;

    boost::split(results, comm, [](char c) { return c == ' '; });
    s1 = results[0];
    if (results.size() > 1) {
        s2 = results[1];
    }
}

int main(int argc, char *argv[]) {
    int sockfd, n, ret;
    char buffer[BUFLEN];
    char message[BUFLEN];
    int choosed_number = 0;
    int vieti = 5;

    if (argc < 3) {
        usage(argv[0]);
    }

    int portno = atoi(argv[2]);
    sockfd = open_connection(argv[1], portno, AF_INET, SOCK_STREAM, 0);

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
            strtok(buffer, "\n");
            string s = buffer;
            string s1, s2;
            tokenize(s, s1, s2);
            int number = atoi(s2.c_str());

            if (!strcmp(buffer, "quit\n")) {
                break;
            } else if (!strncmp(buffer, SET, strlen(SET))) {
                // trimit la server numarul care trebuie ghicit
                n = send(sockfd, buffer, strlen(buffer), 0);
                DIE(n < 0, "send");
            } else if (!strncmp(buffer, GUESS, strlen(GUESS))) {
                if (!choosed_number) {
                    cout << "Nu s-a ales un numar pe care sa il ghicesti\n";
                    continue;
                }
                if (number < 0 || number > 10000) {
                    cout << "Numarul nu este in range\n";
                    continue;
                }
                //. trimit la server incercarea
                n = send(sockfd, s2.c_str(), strlen(buffer), 0);
                DIE(n < 0, "send");

            } else if (!strncmp(buffer, STATUS, strlen(STATUS))) {
                printf("Mai ai %d vieti!\n", vieti);

            } else {
                cout << "Invalid command\n";
            }
        }

        if (FD_ISSET(sockfd, &tmp_fds)) {
            memset(buffer, 0, BUFLEN);
            int rec_msg = recv(sockfd, buffer, sizeof(buffer), 0);

            if (rec_msg == 0) {
                printf("Server has closed!\n");
                break;
            }
            if (rec_msg < 0) {
                perror("Can't receive message from server");
            }

            if (!strncmp(buffer, WRONG, strlen(WRONG))) {
                printf("Raspuns: %s \n", buffer);
                vieti--;
                if (vieti == 0) {
                    printf("Ai pierdut!\n");
                    break;
                } else {
                    printf("Raspuns gresit! Mai ai %d vieti :D\n", vieti);
                }
            } else if (!strncmp(buffer, CHOOSED, strlen(CHOOSED))) {
                printf("S-a ales numarul\n");
                choosed_number = 1;
            } else if (!strncmp(buffer, ACCESS, strlen(ACCESS))) {
                printf("Nu ai voie sa alegi numarul!");
            } else {
                printf("%s\n", RIGHT);
            }
        }
    }

    close(sockfd);

    return 0;
}
