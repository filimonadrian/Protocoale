// #include "arp_parser.h"
#include "skel.h"

// get pair from arp_table which has this ip
struct arp_entry *get_arp_entry(uint32_t ip) {
   
	for (int i = 0; i < arp_table_len; i++){
		if (arp_table[i].ip == ip){
			return &arp_table[i];
		}
	}

    return NULL;
}

void parse_arp_table() {

	FILE *f;
	f = fopen("arp_table.txt", "r");
    if (f == NULL){
        perror("Failed to open arp_table.txt");
    }
	char line[100];
	int i = 0;
	for(i = 0; fgets(line, sizeof(line), f); i++) {
		char ip_str[50], mac_str[50];
		sscanf(line, "%s %s", ip_str, mac_str);
		//fprintf(stderr, "IP: %s MAC: %s\n", ip_str, mac_str);
		arp_table[i].ip = inet_addr(ip_str);
		int rc = hwaddr_aton(mac_str, arp_table[i].mac);
		if (rc < 0){
            perror("Invalid MAC");
        }
	}
	
	arp_table_len = i;
	fclose(f);

}

void complete_arp_pack(struct arpPkt *arp_pkt, uint16_t operation){

	arp_pkt->arphdr.hw_type = htons(ETHERNET);
	arp_pkt->arphdr.prot_type = htons(PROT_IP);
	arp_pkt->arphdr.hw_len = HW_ADDR_LEN;
	arp_pkt->arphdr.prot_len = IP_ADDR_LEN;
	arp_pkt->arphdr.arp_operation = htons(operation);

}

void update_arp_table(arpPkt *arp_pkt){

	/*add or update an entry in my arp_table array */
	struct arp_entry *update_entry = get_arp_entry(arp_pkt->sender_ip_addr);
	if (update_entry != NULL){
		// update_entry->ip = arp_pkt->sender_ip_addr;
		memcpy(update_entry->mac, arp_pkt->sender_hw_addr, HW_ADDR_LEN);
	} else {
		arp_table[arp_table_len].ip = arp_pkt->sender_ip_addr;
		memcpy(arp_table[arp_table_len].mac, arp_pkt->sender_hw_addr, HW_ADDR_LEN);
		arp_table_len++;

	}

}



