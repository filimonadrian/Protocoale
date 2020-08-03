#include "helpers.h"


void usage(char *file) {
    fprintf(stderr, "Usage: %s server_address server_port\n", file);
    exit(0);
}

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

int find_name(vector<client> clients, char *name) {
	for (int i = 0; i < clients.size(); ++i) {
		if (!strcmp(clients[i].name, name)) {
			return i;
		}
	}
	return -1;
}

int main(int argc, char *argv[]) {
    int sockfd, n, ret;
    struct sockaddr_in serv_addr;
    char buffer[BUFLEN];
    char message[BUFLEN];
	char *aux;
	vector<client> clients;
	client c;

    if (argc < 3) {
        usage(argv[0]);
    }

	int portno = atoi(argv[2]);
	sockfd = open_connection(argv[1], portno, AF_INET, SOCK_STREAM, 0);
	memset(buffer, 0, BUFLEN);
	cin >> buffer;
	send(sockfd, buffer, BUFLEN, 0);


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
            } else if (!strcmp(buffer, "CONNECT\n")) {
			
				memset(buffer, 0, BUFLEN);
				cout << "doriti sa va conectati cu: ";
				cin >> buffer;				
				int index = find_name(clients, buffer);
				if (index < 0) {
					printf("Nu exista acest user\n");
					continue;
				}
				printf("%s\n", buffer);
				// int client_sock = open_connection()
			} else if (!strcmp(buffer, "LIST\n")) {
				memset(buffer, 0, BUFLEN);
				strcpy(buffer, "LIST");
				ret = send(sockfd, buffer, strlen(buffer), 0);
				if (ret < 0) {
					perror("Nu se poate trimite operatiunea la server");
				}
			}
            // se trimite mesaj la server
            // n = send(sockfd, buffer, strlen(buffer), 0);
            // DIE(n < 0, "send");
        }

        if (FD_ISSET(sockfd, &tmp_fds)) {

            memset(buffer, 0, BUFLEN);
            int rec_msg = recv(sockfd, buffer, sizeof(buffer), 0);
			if (rec_msg == 0) {
				break;
			}
            DIE(rec_msg < 0, "receive message from server");

			memcpy(&c, buffer, sizeof(client));
			clients.push_back(c);
            printf("%s %s %d\n", c.name, c.ip, c.port);
        }
    }

    close(sockfd);

    return 0;
}
