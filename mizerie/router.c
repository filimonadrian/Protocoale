#include "skel.h"

int interfaces[ROUTER_NUM_INTERFACES];
struct route_table_entry *rtable;
int rtable_size;

struct arp_entry *arp_table;
int arp_table_len;

/*
 Returns a pointer (eg. &rtable[i]) to the best matching route
 for the given dest_ip. Or NULL if there is no matching route.
*/
struct route_table_entry *get_best_route(uint32_t dest_ip) {
	/* TODO 1: Implement the function */
	int max = 0, max_index;
	for (int i = 0; i < rtable_size; i++){
		if ((dest_ip & rtable[i].mask) == rtable[i].prefix){
			if (max < rtable[i].mask){
				max = rtable[i].mask;
				max_index = i;
			}
		}
	}
	if (max == -1)
		return NULL;
		else 
			return &rtable[max_index];
}


/*
 Returns a pointer (eg. &arp_table[i]) to the best matching ARP entry.
 for the given dest_ip or NULL if there is no matching entry.
*/
struct arp_entry *get_arp_entry(uint32_t ip) {
    /* TODO 2: Implement */
	for (int i = 0; i < arp_table_len; i++){
		if (arp_table[i].ip == ip){
			return &arp_table[i];
		}
	}

    return NULL;
}

int main(int argc, char *argv[])
{
	packet m;
	int rc;

	init();
	rtable = malloc(sizeof(struct route_table_entry) * 65000);
	arp_table = malloc(sizeof(struct  arp_entry) * 100);
	DIE(rtable == NULL, "memory");
	rtable_size = read_rtable(rtable);
	parse_arp_table();
	/* Students will write code here */

	while (1) {
		rc = get_packet(&m);
		DIE(rc < 0, "get_message");
		struct ether_header *eth_hdr = (struct ether_header *)m.payload;
		struct iphdr *ip_hdr = (struct iphdr *)(m.payload + sizeof(struct ether_header));
		
		fprintf(stderr, "Eroare Todo 3");


		/* TODO 3: Check the checksum */
		if (ip_checksum(ip_hdr, sizeof (struct iphdr)) != 0){
			continue;
		}
		/* TODO 4: Check TTL >= 1 */
		if (ip_hdr->ttl < 1){
			continue;
		}

		/* TODO 5: Find best matching route (using the function you wrote at TODO 1) */

		fprintf(stderr, "Eroare Todo 3");

		struct route_table_entry *best_route;
		best_route = get_best_route(ip_hdr->daddr);
		if (best_route == NULL){
			continue;
		}

		/* TODO 6: Update TTL and recalculate the checksum */

		fprintf(stderr, "Eroare Todo 3");


		(ip_hdr->ttl)--;
		ip_hdr->check = 0;
		ip_hdr-> check = ip_checksum(ip_hdr, sizeof(struct iphdr));
		
		/* TODO 7: Find matching ARP entry and update Ethernet addresses */


		struct arp_entry *macForEth;
		macForEth = get_arp_entry(ip_hdr->daddr);
		//eth_hdr->ether_dhost = macForEth;
		memcpy(eth_hdr->ether_dhost, macForEth->mac, sizeof(macForEth->mac));

		/* TODO 8: Forward the pachet to best_route->interface */
		send_packet(best_route->interface, &m);

	}
}
