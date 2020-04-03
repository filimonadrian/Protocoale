/*
*  	Protocoale de comunicatii: 
*  	Laborator 6: UDP
*	client mini-server de backup fisiere
*/

#include <fcntl.h>
#include <netinet/in.h>
#include <netdb.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <string.h>
#include <unistd.h>

#include "helpers.h"

void usage(char*file)
{
	fprintf(stderr,"Usage: %s ip_server port_server file\n",file);
	exit(0);
}

/*
*	Utilizare: ./client ip_server port_server nume_fisier_trimis
*/
int main(int argc,char**argv)
{
	if (argc!=4)
		usage(argv[0]);
	
	int fd;
	struct sockaddr_in to_station;
	char buf[BUFLEN];


	/*Deschidere socket*/
	int s = socket(PF_INET, SOCK_DGRAM, 0);
	if (s < 0)
		perror("socket creation failed");
	
	/*Setare struct sockaddr_in pentru a specifica unde trimit datele*/
	to_station.sin_family = AF_INET;
	to_station.sin_port = htons(12345);

	inet_aton("127.0.0.1", &to_station.sin_addr);


	/* Deschidere fisier pentru citire */
	DIE((fd=open(argv[3],O_RDONLY))==-1,"open file");
	strcpy(buf, "ana are mere mari si rosii");
	int size_of_data = 0;
	while ((size_of_data = read(fd, buf, BUFLEN)) ){
		/*
		*  cat_timp  mai_pot_citi
		*		citeste din fisier
		*		trimite pe socket
		*/	
		int se = sendto(s, buf, size_of_data, 0, (struct sockaddr*) &to_station, sizeof(to_station));
		if (se < 0){
			perror("Eroare la trimitere");
		}
		usleep( 10 );
	 }

	/*Inchidere socket*/
	close(s);
	
	/*Inchidere fisier*/
	close(fd);

	return 0;
}
