#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>

#include "lib.h"

#define HOST "127.0.0.1"
#define PORT 10000

int xor_sum(char *buf, short len){
  int result = 0;
  
  for (int i = 0; i < len; i++){
    result = result + (result ^ buf[i]); 
  }
  return result;

}

int main(int argc, char *argv[])
{
	msg t;
	pkt p;
	int res;

	printf("[SENDER] Starting.\n");	
	init(HOST, PORT);

	/* printf("[SENDER]: BDP=%d\n", atoi(argv[1])); */

	//BDP = speed * delay
	int BDP = atoi(argv[1]);
	int window_size = BDP * 1000 / (sizeof(msg) * 8);
	//printf("%d ", window_size);

	/*type : 0 = mesaj
			1 - ACK
			2 - NACK
			*/

	char mesaj[] = "DLE?????????????????DLEDLE??????????????????????DLE";
	
	p.type = 0;
		
	for (int i = 0; i < window_size; i++){
		//construiesc mesajul
		//trimit mesajele
		
		p.type = 0;
		p.checksum = 0;
		strcpy(p.payload, mesaj);

		memcpy(t.payload, &p, MSGSIZE);
		t.len = strlen(p.payload) + 2 * sizeof(int);
		t.parity = '0';
		for (int k = 0; k < t.len; k++){
			for (int j = 0; j < 8; j++){
				t.parity ^= (1 << j) & t.payload[k];
			}
		}

		// send msg 
		res = send_message(&t);
		if (res < 0) {
			perror("[SENDER] Send error. Exiting.\n");
			return -1;
		}
	}

	for (int i = 0; i < COUNT - window_size; i++){
		//primesc ACK
		res = recv_message(&t);
		if (res < 0) {
			perror("[SENDER] Receive error. Exiting.\n");
			return -1;
		}

		memcpy(&p, t.payload, MSGSIZE);
		printf("[send] %s\n", p.payload);

	
		//construiesc mesajul
		//trimit mesajele
		
		p.type = 0;
		p.checksum = 0;
		strcpy(p.payload, mesaj);

		memcpy(t.payload, &p, MSGSIZE);
		t.len = strlen(p.payload) + 2 * sizeof(int);
		t.parity = '0';
		for (int k = 0; k < t.len; k++){
			for (int j = 0; j < 8; j++){
				t.parity ^= (1 << j) & t.payload[k];
			}
		}
		res = send_message(&t);
		if (res < 0) {
			perror("[SENDER] Send error. Exiting.\n");
			return -1;
		}

	}

	for (int i = 0; i < window_size; i++){
		//recv lost ACK
		res = recv_message(&t);
		if (res < 0) {
			perror("[SENDER] Receive error. Exiting.\n");
			return -1;
		}
		
		memcpy(&p, t.payload, MSGSIZE);
		printf("[send] %s\n", p.payload);

	}

/*
	for (i = 0; i < COUNT; i++) {
		// cleanup msg
		memset(&t, 0, sizeof(msg));
		
		strcpy(t.payload, "HAMGHAM");
		// gonna send an empty msg 
		t.len = MSGSIZE;

		
		// send msg
		res = send_message(&t);
		if (res < 0) {
			perror("[SENDER] Send error. Exiting.\n");
			return -1;
		}
		
		// wait for ACK 
		res = recv_message(&t);
		if (res < 0) {
			perror("[SENDER] Receive error. Exiting.\n");
			return -1;
		}
	}
	*/

	printf("[SENDER] Job done, all sent.\n");
		
	return 0;
}
