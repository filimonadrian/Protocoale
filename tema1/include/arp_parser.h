
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>           // close()
#include <string.h>           // strcpy, memset(), and memcpy()

#include <netdb.h>            // struct addrinfo
#include <sys/types.h>        // needed for socket(), uint8_t, uint16_t
#include <sys/socket.h>       // needed for socket()
#include <netinet/in.h>       // IPPROTO_RAW, INET_ADDRSTRLEN
#include <netinet/ip.h>       // IP_MAXPACKET (which is 65535)
#include <arpa/inet.h>        // inet_pton() and inet_ntop()
#include <sys/ioctl.h>        // macro ioctl is defined
#include <bits/ioctls.h>      // defines values for argument "request" of ioctl.
#include <net/if.h>           // struct ifreq
#include <linux/if_ether.h>   // ETH_P_ARP = 0x0806
#include <linux/if_packet.h>  // struct sockaddr_ll (see man 7 packet)
#include <net/ethernet.h>

#include <errno.h>            // errno, perror()

#include <netinet/if_ether.h> // struct ether_arp 
#include <net/if_arp.h> // struct arphdr 

#include <sys/select.h>
/* ethheader */
#include <net/ethernet.h>
/* ether_header */
#include <arpa/inet.h>
/* icmphdr */
#include <netinet/ip_icmp.h>
#include <asm/byteorder.h>

//define-urile exista si in <net/if_arp.h>
#define ETHERNET 1

#define PROT_IP 0x800
#define PROT_ARP 0x806

#define HW_ADDR_LEN 6
#define IP_ADDR_LEN 4

#define ARP_REQUEST 1
#define ARP_REPLY 2

#define ARP_PACKET_LEN sizeof(struct arpPkt)

// #define BROADCAST {0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF};

// am redefinit structurile arphdr si ether_arp 
// deoarece campurile nu aveau nume sugestive

typedef struct arpHdr{

    uint16_t hw_type;				// Format of hardware address.  
    uint16_t prot_type;				// Format of protocol address.  
    uint8_t hw_len;					// Length of hardware address.  
    uint8_t prot_len;				// Length of protocol address.  
	uint16_t arp_operation;			// ARP opcode (command).  
}arpHdr;

typedef struct arpPkt {
	struct arpHdr arphdr;					// fixed-size header 
	uint8_t sender_hw_addr[HW_ADDR_LEN];	// sender hardware address 
	uint32_t sender_ip_addr;				// sender ip address 
	uint8_t target_hw_addr[HW_ADDR_LEN];	// target hardware address
	uint32_t target_ip_addr;				// target ip address 
}arpPkt;


/*
struct	ether_arp {
	struct	arphdr ea_hdr;			// fixed-size header 
	uint8_t arp_sha[ETH_ALEN];		// sender hardware address 
	uint8_t arp_spa[4];				// sender protocol adressd
	uint8_t arp_tha[ETH_ALEN];		// target hardware address 
	uint8_t arp_tpa[4];				// target protocol address 
};
*/

struct arp_entry {
	uint32_t ip;
	uint8_t mac[6];
};

struct arp_entry *arp_table;
int arp_table_len;

extern int hwaddr_aton(const char *txt, uint8_t *addr);

void parse_arp_table();
struct arp_entry *get_arp_entry(uint32_t ip);
void complete_arp_pack(struct arpPkt *arp_pkt, uint16_t operation);
void update_arp_table(arpPkt *arp_pkt);

     


