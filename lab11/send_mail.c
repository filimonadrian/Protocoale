/*
 * Protocoale de comunicatii
 * Laborator 11 - E-mail
 * send_mail.c
 */

#include <arpa/inet.h>
#include <errno.h>
#include <fcntl.h>
#include <netinet/in.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>

#define SMTP_PORT 25
#define MAXLEN 500

/**
 * Citeste maxim maxlen octeti de pe socket-ul sockfd in
 * buffer-ul vptr. Intoarce numarul de octeti cititi.
 */
ssize_t read_line(int sockd, void *vptr, size_t maxlen) {
    ssize_t n, rc;
    char c, *buffer;

    buffer = vptr;

    for (n = 1; n < maxlen; n++) {
        if ((rc = read(sockd, &c, 1)) == 1) {
            *buffer++ = c;

            if (c == '\n') {
                break;
            }
        } else if (rc == 0) {
            if (n == 1) {
                return 0;
            } else {
                break;
            }
        } else {
            if (errno == EINTR) {
                continue;
            }

            return -1;
        }
    }

    *buffer = 0;
    return n;
}

/**
 * Trimite o comanda SMTP si asteapta raspuns de la server. Comanda
 * trebuie sa fie in buffer-ul sendbuf. Sirul expected contine
 * inceputul raspunsului pe care trebuie sa-l trimita serverul
 * in caz de succes (de exemplu, codul 250). Daca raspunsul
 * semnaleaza o eroare, se iese din program.
 */
void send_command(int sockfd, const char sendbuf[], char *expected) {
    char recvbuf[MAXLEN];
    int nbytes;
    char CRLF[2] = {13, 10};

    printf("Trimit: %s\n", sendbuf);
    write(sockfd, sendbuf, strlen(sendbuf));
    write(sockfd, CRLF, sizeof(CRLF));

    nbytes = read_line(sockfd, recvbuf, MAXLEN - 1);
    recvbuf[nbytes] = 0;
    printf("Am primit: %s", recvbuf);

    if (strstr(recvbuf, expected) != recvbuf) {
        printf("Am primit mesaj de eroare de la server!\n");
        exit(-1);
    }
}

void read_file(char contents[], char file_name[]) {
    int fd, check;
    
    fd = open(file_name, O_RDONLY);
    while ((check = read(fd, contents, sizeof(contents)))) {
        if (check < 0) {
            perror("Can't read from file");
        }
    }
}

int main(int argc, char **argv) {
    int sockfd;
    int port = 465;

    struct sockaddr_in servaddr;
    char server_ip[INET_ADDRSTRLEN];
    char sendbuf[MAXLEN];
    char recvbuf[MAXLEN];

    if (argc != 3) {
        printf("Utilizare: ./send_msg adresa_server nume_fisier\n");
        exit(-1);
    }

    strncpy(server_ip, argv[1], INET_ADDRSTRLEN);

    // TODO 1: creati socket-ul TCP client

    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    /*

*/
    // TODO 2: completati structura sockaddr_in cu
    // portul si adresa IP ale serverului SMTP
    inet_aton(argv[1], &servaddr.sin_addr);

    servaddr.sin_family = AF_INET;
    servaddr.sin_port = htons(port);

    // TODO 3: conectati-va la server

    int ret = connect(sockfd, (struct sockaddr *)&servaddr,
                      sizeof(servaddr));
    if (ret < 0) {
        perror("Can't connect to server");
    }

    // se primeste mesajul de conectare de la server
    read_line(sockfd, recvbuf, MAXLEN - 1);
    printf("Am primit: %s\n", recvbuf);

    // se trimite comanda de HELO
    sprintf(sendbuf, "HELO Adrian");
    send_command(sockfd, sendbuf, "250");

    // se trimite comanda de AUTH_LOGIN
    sprintf(sendbuf, "AUTH LOGIN");
    send_command(sockfd, sendbuf, "334");

    // se trimite user
    sprintf(sendbuf, "ZDlkOTRlM2RkY2ZlNmY=");
    send_command(sockfd, sendbuf, "334");

    // se trimite parola
    sprintf(sendbuf, "ZmJhNTJlYzI4ZWM1ZjY=");
    send_command(sockfd, sendbuf, "235");

    // TODO 4: trimiteti comanda de MAIL FROM

    sprintf(sendbuf, "MAIL FROM: <adrian.filimon@yahoo.com>");
    send_command(sockfd, sendbuf, "250");

    // TODO 5: trimiteti comanda de RCPT TO

    sprintf(sendbuf, "RCPT TO: <radu.ciobanu@yahoo.com>");
    send_command(sockfd, sendbuf, "250");

    // TODO 6: trimiteti comanda de DATA

    sprintf(sendbuf, "DATA");
    send_command(sockfd, sendbuf, "354");

    // TODO 7: trimiteti e-mail-ul (antete + corp + atasament)

    strcpy(sendbuf, "MIME-Version: 1.0\r\n");
    strcat(sendbuf, "From: adrian.filmon@gmail.com\r\n");
    strcat(sendbuf, "To: radu.ciobanu@mail.com\r\n");
    strcat(sendbuf, "Subject: Mail\r\n");
    strcat(sendbuf, "Content-Type: multipart/mixed; boundary=xxx\r\n");
    strcat(sendbuf, "\r\n");
    strcat(sendbuf, "--xxx\r\n");
    strcat(sendbuf, "Content-Type: text/plain\r\n");
    strcat(sendbuf, "\r\n");
    strcat(sendbuf, "This is the body of the e-mail.\r\n");
    strcat(sendbuf, "Best regards\r\n");
    strcat(sendbuf, "\r\n");
    strcat(sendbuf, "--xxx\r\n");
    strcat(sendbuf, "Content-Type: text/plain\r\n");
    strcat(sendbuf, "Content-Disposition: attachment; filename=\"file.txt\"\r\n");
    strcat(sendbuf, "\r\n");
    strcat(sendbuf, "This is an attachment\r\n");
    strcat(sendbuf, "\r\n");
    strcat(sendbuf, "--xxx\r\n");
    strcat(sendbuf, ".");

    send_command(sockfd, sendbuf, "250");
    // TODO 8: trimiteti comanda de QUIT
    sprintf(sendbuf, "QUIT");
    send_command(sockfd, sendbuf, "221");

    // TODO 9: inchideti socket-ul TCP client
    close(sockfd);
    return 0;
}
