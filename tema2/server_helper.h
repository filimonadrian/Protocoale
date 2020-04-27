#ifndef SERVER_HELPER_H
#define SERVER_HELPER_H

#include <iostream>
#include <unordered_map>
#include <vector>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

void close_all_sock(fd_set *fd, int fdmax);
void usage(char *file);
static inline void clear_input_buffer();
bool close_server(fd_set *read_fds, int fdmax);



#endif