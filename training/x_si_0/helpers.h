#ifndef _HELPERS_H
#define _HELPERS_H 1

#include <stdio.h>
#include <stdlib.h>

/*
 * Macro de verificare a erorilor
 * Exemplu:
 *     int fd = open(file_name, O_RDONLY);
 *     DIE(fd == -1, "open failed");
 */

#define DIE(assertion, call_description)	\
	do {									\
		if (assertion) {					\
			fprintf(stderr, "(%s, %d): ",	\
					__FILE__, __LINE__);	\
			perror(call_description);		\
			exit(EXIT_FAILURE);				\
		}									\
	} while(0)

#define BUFLEN		10	// dimensiunea maxima a calupului de date
#define MAX_CLIENTS	100	// numarul maxim de clienti in asteptare
#define x 1
#define o 0
#define blank 3
#define BOARD_SIZE 3

typedef struct player {
	// socket for both players
	int player1;
	int player2;
	int turn;
}player;



#endif
