/*
*  	Protocoale de comunicatii: 
*  	Laborator 6: UDP
*	mini-server de backup fisiere
*/

#include <fcntl.h>
#include <netinet/in.h>
#include <netdb.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>

#include "helpers.h"


void usage(char*file)
{
	fprintf(stderr,"Usage: %s server_port file\n",file);
	exit(0);
}

/*
*	Utilizare: ./server server_port nume_fisier
*/
int main(int argc,char**argv)
{
	int fd;

	if (argc!=3)
		usage(argv[0]);
	
	struct sockaddr_in my_sockaddr, from_station ;
	char buf[BUFLEN];


	/*Deschidere socket*/
	int s = socket(PF_INET, SOCK_DGRAM, 0);
	if (s < 0)
		perror("socket creation failed");
	
	/*Setare struct sockaddr_in pentru a asculta pe portul respectiv */
	my_sockaddr.sin_family = AF_INET;
	my_sockaddr.sin_port = htons(12345);

	
	/* Legare proprietati de socket */

	my_sockaddr.sin_addr.s_addr = INADDR_ANY;

	//"anunt" ca astept sa primesc date 
	bind(s, (struct sockaddr*) &my_sockaddr, sizeof(my_sockaddr));
	if (bind < 0){
		perror("Nu s-a facut bind-ul");
	}
	
	/* Deschidere fisier pentru scriere */
	DIE((fd=open(argv[2],O_WRONLY|O_CREAT|O_TRUNC,0644))==-1,"open file");
	
	int recv_test = 1;
	int write_test = 1;
	while(recv_test){
		/*
		*  cat_timp  mai_pot_citi
		*		citeste din socket
		*		pune in fisier
		*/
		//ii dau o valoare random
		//si pun in el
		socklen_t length = 10;
		recv_test = recvfrom(s, buf, BUFLEN, 0, (struct sockaddr*) &from_station, &length);
		if (recv_test < 0){
			perror("Eroare la receptie");
		}

		// printf("%s", buf);
		write_test = write(fd, buf, recv_test);
		if (write_test < 0)
			perror("eraore la scrierea datelor de catre server");
		
	}

	/*Inchidere socket*/	
	close(s);
	
	/*Inchidere fisier*/
	close(fd);

	return 0;
}
