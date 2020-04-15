// Protocoale de comunicatii
// Laborator 9 - DNS
// dns.c

#include <stdlib.h>
#include <stdio.h>
#include <sys/types.h>
#include <unistd.h>
#include <string.h>
#include <sys/socket.h>
#include <netdb.h>
#include <arpa/inet.h>

int usage(char* name)
{
	printf("Usage:\n\t%s -n <NAME>\n\t%s -a <IP>\n", name, name);
	return 1;
}

// Receives a name and prints IP addresses
void get_ip(char* name)
{
	int ret;
	struct addrinfo hints, *result, *p;

	// TODO: set hints
	hints.ai_flags = AI_PASSIVE | AI_CANONNAME;
	hints.ai_family = AF_UNSPEC;
	hints.ai_socktype = SOCK_DGRAM;
	hints.ai_protocol = IPPROTO_UDP;

	// TODO: get addresses
	ret = getaddrinfo(name, NULL, &hints, &result);
	if (ret < 0) {
		gai_strerror(ret);
	}

	// TODO: iterate through addresses and print them

	p = result;

	while (p) {
		if (p->ai_family == AF_INET) {
			char ip[INET_ADDRSTRLEN];
			struct sockaddr_in *addr = (struct sockaddr_in*) p->ai_addr;
			inet_ntop (p->ai_family, &(addr->sin_addr), ip, sizeof(ip));


			printf("ip %s port %d canon_name %s socktype %d\n", ip, ntohs(addr->sin_port), p->ai_canonname, p->ai_socktype);
			// afisare:
			// ip 172.217.20.14 port 0 canon_name (null) socktype 2

		} else if (p->ai_family == AF_INET6) {
			char ip[INET6_ADDRSTRLEN];
			struct sockaddr_in6 *addr = (struct sockaddr_in6*) p->ai_addr;

			inet_ntop (p->ai_family, &(addr->sin6_addr), ip, sizeof(ip));
			printf("ip %s port %d canon_name %s socktype %d\n", ip, ntohs(addr->sin6_port), p->ai_canonname, p->ai_socktype);

		} 
		p = p->ai_next;
	}

	// TODO: free allocated data

	freeaddrinfo(result);
}

// Receives an address and prints the associated name and service
void get_name(char* ip)
{
	int ret;
	struct sockaddr_in addr;
	char host[1024];
	char service[20];

	// TODO: fill in address data
	addr.sin_family = AF_INET;
	inet_aton(ip, &addr.sin_addr);

	// TODO: get name and service
	ret = getnameinfo((struct sockaddr*) &addr, sizeof(addr), 
					host, sizeof(host), service, sizeof(service), 0);
	if (ret < 0) {
		gai_strerror(ret);
	}

	// TODO: print name and service
	printf("Name %s Service %s\n", host, service);

}

int main(int argc, char **argv)
{
	if (argc < 3) {
		return usage(argv[0]);
	}

	if (strncmp(argv[1], "-n", 2) == 0) {
		get_ip(argv[2]);

	} else if (strncmp(argv[1], "-a", 2) == 0) {
		get_name(argv[2]);

	} else {
		return usage(argv[0]);
	}

	return 0;
}
