#ifndef SERVER_H
#define SERVER_H

#include "server_helper.h"
#include <arpa/inet.h>
#include <iostream>
#include <netinet/in.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>
#include <unordered_map>
#include <vector>
// #include "helpers.h"

using namespace std;

#define BUFLEN 256
#define MAX_CLIENTS 5

#define ONLINE 1
#define OFFLINE 0

#define SUBSCRIBE 1
#define UNSUBSCRIBE 0
#define ID_ACKNOWLEDGE 2
#define SF_ON 1
#define SF_OFF 0

typedef struct tcp_msg {
    char client_id[11];
    char payload[1500];
    uint8_t type;
    uint8_t SF;
} tcp_msg;

typedef struct udp_msg {

} udp_msg;

typedef struct tcp_client {
    char client_id[11];
    uint32_t sockfd;
    uint8_t state;

} tcp_client;

typedef struct udp_client {

} udp_client;

typedef struct mesaj {
    int sockfd;
    int name;
} mesaj;

// map with <topic_name> <all_subscribers_for_this_topic>
// unordered_map<string, vector<tcp_client>> forum_tcp;
// unordered_map<string, vector<tcp_client>> forum_udp;


#endif