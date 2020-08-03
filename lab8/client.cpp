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


int main(int argc, char *argv[]) {
    int sockfd, n, ret;
    char buffer[BUFLEN];
    char message[BUFLEN];

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

            if (!strcmp(buffer, "exit\n")) {
                break;
            }

            // se trimite mesaj la server
            n = send(sockfd, buffer, strlen(buffer), 0);
            DIE(n < 0, "send");
        }

        if (FD_ISSET(sockfd, &tmp_fds)) {

            int rec_msg = recv(sockfd, message, sizeof(message), 0);

            if (rec_msg == 0) {
				printf("Server has closed!\n");
                break;
            }
            if (rec_msg < 0) {
                perror("Can't receive message from server");
            }
            printf("rec: %s \n", message);
        }
    }

    close(sockfd);

    return 0;
}
