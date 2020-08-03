#ifndef _HELPERS_H
#define _HELPERS_H 1

#include <arpa/inet.h>
#include <netdb.h>
#include <netinet/in.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>
#include<iostream>

#include <vector>
#include <string>

using namespace std;
#include <boost/algorithm/string/split.hpp>

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

#define BUFLEN		1024	// dimensiunea maxima a calupului de date
#define MAX_CLIENTS	20	// numarul maxim de clienti in asteptare


const char *ADD_ADDRESS = "add_address ";
const char *ADD_NAME = "add_name ";
const char *ADD_MAIL = "add_mail ";
const char *GET_ADDRESS = "get_address ";
const char *GET_NAME = "get_name ";
const char *GET_MAIL = "get_mail ";
const char *NO_ENTRY = "NO ENTRY FOR THAT!!";

const char *STATUS = "status ";
const char *SET = "set ";
const char *GUESS = "guess ";
const char *WRONG = "WRONG!";
const char *RIGHT = "Felicitari! Numarul a fost ghicit! Ne revedem curand!\n";
const char *CHOOSED = "CHOOSED";
const char *ACCESS = "ACCESS";









#endif
