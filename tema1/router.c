#include "skel.h"


void complete_ip_icmp(struct iphdr *ip_icmp, struct iphdr* ip_hdr, int len){
				ip_icmp->version = 4;
				ip_icmp->ihl = 5;
				ip_icmp->tos = 0;
				ip_icmp->tot_len = htons(len - sizeof(struct ether_header));
				ip_icmp->id = htons(ip_hdr->id);                       
				ip_icmp->frag_off = 0;
				ip_icmp->ttl = ip_hdr->ttl;
				ip_icmp->protocol = IPPROTO_ICMP;
			
				ip_icmp->saddr = ip_hdr->daddr;
				ip_icmp->daddr = ip_hdr->saddr;

				ip_icmp->check = 0;
				ip_icmp->check = checksum(ip_icmp, sizeof(struct iphdr));

	}


int main(int argc, char *argv[]){	
	setvbuf(stdout, NULL, _IONBF, 0);
	packet m;
	int rc;

	init();
	rtable = malloc(sizeof(struct route_table_entry) * 65000);
	arp_table = malloc(sizeof(struct arp_entry) * 100);

	DIE(rtable == NULL, "memory");
	rtable_size = parse_route_table();
	parse_arp_table();

	qsort (rtable, rtable_size, sizeof(struct route_table_entry), compare_func);

	while (1) {

		rc = get_packet(&m);
		DIE(rc < 0, "get_message");

		struct ether_header *eth_hdr = (struct ether_header *)m.payload;


		if (ntohs(eth_hdr->ether_type) == ETHERTYPE_ARP){
			struct arpPkt *arp_pkt = (struct arpPkt *)
								(m.payload + ETHER_HEADER_LEN);
			
			// ip address of interface from which packet comes
			uint32_t local_ip = inet_addr(get_interface_ip(m.interface));

			/* if ARP packet is for me, i'll process it */
			/* if it's a reply, i'll update arp_table */

			if (ntohs(arp_pkt->arphdr.arp_operation) == ARP_REPLY){

				if (local_ip == ntohl(arp_pkt->target_ip_addr)){
					update_arp_table(arp_pkt);
				}

			/*CHECK IF THIS REPLY SOLVE PACKETS FROM THE QUEUE */

				/*else if it's request, search in interfaces table 
				check if i have an ip_addr equals to received ip
				get mac for that ip addr and send reply packet
				*/ 
			} else if (ntohs(arp_pkt->arphdr.arp_operation) == ARP_REQUEST){
				fprintf(stderr, "I've received a request\n");
				 					

				 if (ntohl(arp_pkt->target_ip_addr) == local_ip){
					fprintf(stderr, "It's my ip\n");
					

					packet pkt;
					init_packet(&pkt);

					pkt.interface = m.interface;
					pkt.len = ARP_PACKET_LEN + ETHER_HEADER_LEN;

					// destinatia o sa fie de unde am primit pachetul
					// sursa o sa fie mac-ul de pe interfata pe care am primit cererea

					uint8_t local_mac[HW_ADDR_LEN];
					// uint32_t local_ip = inet_addr(get_interface_ip(m.interface));
					get_interface_mac(m.interface, local_mac);

					struct ether_header *eth_frame = (struct ether_header *)pkt.payload;
					struct arpPkt *arp_reply = (struct arpPkt *)(pkt.payload + ETHER_HEADER_LEN);

					memcpy(eth_frame->ether_dhost, eth_hdr->ether_shost, HW_ADDR_LEN);
					memcpy(eth_frame->ether_shost, local_mac, HW_ADDR_LEN);
					eth_frame->ether_type = ETHERTYPE_ARP;

					complete_arp_pack(arp_reply, ARP_REPLY);

					memcpy(arp_reply->sender_hw_addr, local_mac, HW_ADDR_LEN);
					arp_reply->sender_ip_addr = local_ip;
					memcpy(arp_reply->target_hw_addr, eth_hdr->ether_shost, HW_ADDR_LEN);
					arp_reply->target_ip_addr = arp_pkt->sender_ip_addr;

					send_packet(pkt.interface, &pkt);
					continue;
									
				}

			}	
			

		} else if (ntohs(eth_hdr->ether_type) == ETHERTYPE_IP){
			struct iphdr *ip_hdr = (struct iphdr *)(m.payload + ETHER_HEADER_LEN);
			struct icmphdr *icmp_hdr = (struct icmphdr *)(m.payload + 
									ETHER_HEADER_LEN + IP_HEADER_LEN);

		// create a new packet for ICMP request
			packet pkt;
			init_packet(&pkt);
			pkt.len = ETHER_HEADER_LEN + IP_HEADER_LEN + ICMP_HEADER_LEN;
			pkt.interface = m.interface;

			struct ether_header *eth_icmp = (struct ether_header *)pkt.payload;
			struct iphdr *ip_icmp = (struct iphdr *)(pkt.payload + ETHER_HEADER_LEN);
			struct icmphdr *icmp_req = (struct icmphdr *)(pkt.payload + IP_HEADER_LEN + 
															ETHER_HEADER_LEN);


			memcpy(eth_icmp->ether_shost, eth_hdr->ether_dhost, HW_ADDR_LEN);
			memcpy(eth_icmp->ether_dhost, eth_hdr->ether_shost, HW_ADDR_LEN);
			eth_icmp->ether_type = htons(ETHERTYPE_IP);

			complete_ip_icmp(ip_icmp, ip_hdr, pkt.len);

			icmp_req->code = 0;
			icmp_req->un.echo.id = icmp_hdr->un.echo.id;
			icmp_req->un.echo.sequence = htons(icmp_hdr->un.echo.sequence);


			if ((icmp_hdr->type == ICMP_ECHO) && 
						(ip_hdr->daddr == inet_addr(get_interface_ip(m.interface)))){
				icmp_req->type = ICMP_ECHOREPLY;
				icmp_req->checksum = 0;
				icmp_req->checksum = checksum(icmp_req, sizeof(struct icmphdr));
				send_packet(pkt.interface, &pkt);
				continue;
			}

			/*Check the checksum */
			if (checksum(ip_hdr, sizeof (struct iphdr)) != 0){
				continue;
			}
			/* Check TTL >= 1 */
			if (ip_hdr->ttl <= 1){
				// send am icmp message back
				icmp_req->type = ICMP_TIME_EXCEEDED;
				icmp_req->checksum = 0;
				icmp_req->checksum = checksum(icmp_req, sizeof(struct icmphdr));
				send_packet(pkt.interface, &pkt);
				
				continue;
			}


			struct route_table_entry *best_route;
			best_route = get_best_route(ip_hdr->daddr);
			if (best_route == NULL){
				icmp_req->type = ICMP_DEST_UNREACH;
				icmp_req->checksum = 0;
				icmp_req->checksum = checksum(icmp_req, sizeof(struct icmphdr));
				send_packet(pkt.interface, &pkt);

				continue;
			}

			/* Update TTL and recalculate the checksum */

			(ip_hdr->ttl)--;
			ip_hdr->check = 0;
			ip_hdr-> check = checksum(ip_hdr, sizeof(struct iphdr));
			
			/*Find matching ARP entry and update Ethernet addresses */

			struct arp_entry *dest_entry =  get_arp_entry(ip_hdr->daddr);
			if (dest_entry == NULL){
				packet pkt;
				init_packet(&pkt);

				pkt.interface = m.interface;
				pkt.len = ARP_PACKET_LEN + ETHER_HEADER_LEN;

				uint32_t local_ip = inet_addr(get_interface_ip(m.interface));
				uint8_t local_mac[HW_ADDR_LEN];
				uint8_t BROADCAST_mac[HW_ADDR_LEN];
				
				for (int i = 0; i < HW_ADDR_LEN; i++){
					BROADCAST_mac[i] = 255;
				}
				
				
				get_interface_mac(m.interface, local_mac);

				struct ether_header *eth_frame = (struct ether_header *)pkt.payload;
				struct arpPkt *arp_pkt = (struct arpPkt *)(pkt.payload + ETHER_HEADER_LEN);


				memcpy(eth_frame->ether_dhost, BROADCAST_mac, HW_ADDR_LEN);
				memcpy(eth_frame->ether_shost, local_mac, HW_ADDR_LEN);
				eth_frame->ether_type = ETHERTYPE_ARP;
				
				// sender_mac and ip --> router mac and ip (from interface x)
				// mac_target -- broadcast
				// target ip -- ip_destination addres --> ip which mac addr
				// can't be resolved
					
				complete_arp_pack(arp_pkt, ARP_REQUEST);

				memcpy(arp_pkt->sender_hw_addr, local_mac, HW_ADDR_LEN);
				arp_pkt->sender_ip_addr = local_ip;
				memcpy(arp_pkt->target_hw_addr, BROADCAST_mac, HW_ADDR_LEN);
				arp_pkt->target_ip_addr = ip_hdr->daddr;

				send_packet(pkt.interface, &pkt);

				/*PUSH packet m on queue */

				continue;

				// ARP_request -- BROADCAST
				// send message(arp) and push origial packet on queue

			} else {
				memcpy(eth_hdr->ether_dhost, dest_entry->mac, sizeof(dest_entry->mac));
			}

			/* Forward the pachet to best_route->interface */
			send_packet(best_route->interface, &m);

		}

	}
}
