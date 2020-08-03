#include "helpers.h"

void usage(char *file) {
    fprintf(stderr, "Usage: %s server_port\n", file);
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

void read_and_send(char *filename, int sockfd) {
    int fd = open(filename, O_RDONLY);
    int ret = 0, send_test = 0;
    if (fd <= 0) {
        perror("Can't read from this file");
    }
    char buffer[1024];
    printf("Sending page..\n");
    int i = 0;

    while ((ret = read(fd, buffer, sizeof(buffer)))) {

        if (ret < 0) {
            perror("Can't read from this file!");
        }

        if (ret == 0) {
            break;
        }
        // printf("%s", buffer);
        send_test = send(sockfd, buffer, sizeof(buffer), 0);
        if (send_test < 0) {
            perror("Can't send the webpage to the server");
        }
        i += ret;
        memset(buffer, 0, sizeof(buffer));
    }
    printf("%lf KB of data sent\n", i / 1024.0);

    puts("Completed\n");
    close(fd);
}

void get_page(char *link, int port) {
    BIO *bio;
    SSL *ssl;
    SSL_CTX *ctx;

    int p;

    // char *request = "GET / HTTP/1.0\r\nHost: www.verisign.com\r\n\r\n";
    char request[50] = "GET / HTTP/1.0\r\nHost: ";
    strcat(request, link);
    strcat(request, "\r\n\r\n");

    char r[1024];

    /* initializare librarie */

    SSL_library_init();
    ERR_load_BIO_strings();
    SSL_load_error_strings();
    OpenSSL_add_all_algorithms();

    /* TO DO initializare context */
    ctx = SSL_CTX_new(SSLv23_client_method());

    /* incarca trust store */

    if (!SSL_CTX_load_verify_locations(ctx, "TrustStore.pem", NULL)) {
        fprintf(stderr, "Error loading trust store\n");
        ERR_print_errors_fp(stderr);
        SSL_CTX_free(ctx);
        return;
    }

    /* stabileste conexiune */

    bio = BIO_new_ssl_connect(ctx);

    /* Seteaza flag SSL_MODE_AUTO_RETRY  */

    BIO_get_ssl(bio, &ssl);
    SSL_set_mode(ssl, SSL_MODE_AUTO_RETRY);

    /* TO DO setup conexiune */

    // BIO *bio = BIO_new_ssl_connect(ctx);

    // BIO_set_conn_hostname(bio, "www.verisign.com:443");
    char link_with_port[100];
    sprintf(link_with_port, "%s:%d", link, port);

    // BIO_set_conn_hostname(bio, "www.verisign.com:443");
    BIO_set_conn_hostname(bio, link_with_port);

    if (BIO_do_connect(bio) <= 0) {
        perror("connection error");
    }

    /* TO DO verifica certificat */
    if (SSL_get_verify_result(ssl) != X509_V_OK) {
        perror("Certificatul nu este valid");
    }

    /* Trimite request */

    BIO_write(bio, request, strlen(request));

    /* TO DO Citeste raspuns si pregateste  output*/

    int fd = open(link, O_CREAT | O_WRONLY | O_APPEND,
                  S_IWUSR | S_IRUSR | S_IWGRP | S_IROTH | S_IWOTH);
    if (fd < 0) {
        perror("Err to write in file");
    }

    while (BIO_read(bio, r, strlen(r))) {
        write(fd, r, strlen(r));
    }

    /* Inchide conexiune si elibereaza context */

    close(fd);
    BIO_free_all(bio);
    SSL_CTX_free(ctx);

    printf("Received webpage\n");
}

int main(int argc, char *argv[]) {

    int sockfd, newsockfd, portno;
    char buffer[BUFLEN];
    struct sockaddr_in serv_addr, cli_addr;
    int n, i, ret;
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

                        // se scoate din multimea de citire socketul inchis
                        FD_CLR(i, &read_fds);

                    } else {
                        char *link = buffer + 4;

                        printf("%s\n", link);
                        get_page(link, 443);
                        read_and_send(link, i);
                        // printf("S-a primit de la clientul de pe socketul %d mesajul: %s\n", i, buffer);
                    }
                }
            }
        }
    }

    close(sockfd);

    return 0;
}
