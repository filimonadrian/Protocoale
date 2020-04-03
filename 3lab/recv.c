#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>

#include "lib.h"

#define HOST "127.0.0.1"
#define PORT 10001

int main(void)
{
	msg r;
	pkt p;
	int i, res;
	char checksum = '0';

	printf("[RECEIVER] Starting.\n");
	init(HOST, PORT);
	
	for (i = 0; i < COUNT; i++) {
		// wait for message 
		res = recv_message(&r);
		if (res < 0) {
			perror("[RECEIVER] Receive error. Exiting.\n");
			return -1;
		}
		
		checksum = '0';
		for (int k = 0; k < r.len; k++){
			for (int j = 0; j < 8; j++){
				checksum ^= (1 << j) & r.payload[k];
			}
		}

		if (checksum == r.parity){
			memcpy(&p, r.payload, MSGSIZE);
			//printf("%d %s", r.len, r.payload);
			//type - 0 pentru mesaj, 1 pentru ack, 2 pentru nack
			printf("[recv] MESAJUL ESTE <%s>, ", p.payload);
			printf("dimeniunea <%d>, sending ACK...", r.len);
			printf("Checksum-ul este %d \n", r.parity);
			
			//construiesc mesajul
			//trimit mesajele
			
			p.type = 1;
			p.checksum = 0;
			strcpy(p.payload, "ACK"); 

			memcpy(r.payload, &p, MSGSIZE);
			r.len = strlen(p.payload) + 2 * sizeof(int);
		} else {
			p.type = 2;
			p.checksum = 0;
			strcpy(p.payload, "NACK"); 

			memcpy(r.payload, &p, MSGSIZE);
			r.len = strlen(p.payload) + 2 * sizeof(int);
		}
		

		//send message
		res = send_message(&r);
		if (res < 0) {
			perror("[RECEIVER] Send ACK error. Exiting.\n");
			return -1;
		}
	}
	
	
	
	/*
	for (i = 0; i < COUNT; i++) {
		// wait for message 
		res = recv_message(&r);
		printf("%s", r.payload);
		if (res < 0) {
			perror("[RECEIVER] Receive error. Exiting.\n");
			return -1;
		}
		
		// send dummy ACK
		res = send_message(&r);
		if (res < 0) {
			perror("[RECEIVER] Send ACK error. Exiting.\n");
			return -1;
		}
	}
	*/
	printf("[RECEIVER] Finished receiving..\n");
	return 0;
}
