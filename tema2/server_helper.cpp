
// #include "server.h"
#include "server_helper.h"
/* functions to:
if topic it s new, add new topic and client
if topic is already knwown, add the user at the end of user_list
*/

void close_all_sock(fd_set *fd, int fdmax) {
    for (int sock = 0; sock < fdmax; sock++) {
        if (FD_ISSET(sock, fd)) {
            close(sock);
        }
    }
}

void usage(char *file) {
    fprintf(stderr, "Usage: %s server_port\n", file);
    exit(0);
}

static inline void clear_input_buffer() {
    char c = 0;
    while ((c = getchar()) != '\n') {
    }
}

bool close_server(fd_set read_fds, int fdmax) {
    char exit_message[10];
    memset(exit_message, 0, 10);
    fgets(exit_message, 6, stdin);
    if (exit_message[5] == 0 && strncmp(exit_message, "exit", 4) == 0) {
        close_all_sock(&read_fds, fdmax);
        printf("Exiting..\n");
        return true;
    }

    printf("That is not a valid command -_- \n");
    return false;
}
