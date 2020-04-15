#include "route_parser.h"


int compare_func(const void *a, const void *b){
	struct route_table_entry *entry1 = (struct route_table_entry *)a;
	struct route_table_entry *entry2 = (struct route_table_entry *)b;

	int diff = (entry1->prefix - entry2->prefix); 
	
	if (diff == 0){
		if (entry1->mask < entry2->mask){
			return -1;
		} else if (entry1->mask > entry2->mask){
			return 1;
		} else {
			return 0;
		}
	}
	return diff;
}


int parse_route_table() {

	FILE *f;
	f = fopen("rtable.txt", "r");
    if (f == NULL){
        perror("Failed to open rtable.txt");
    }
	char line[100];
	int i = 0;
	for(i = 0; fgets(line, sizeof(line), f); i++) {
		char prefix_str[20], next_hop_str[20], mask_str[20], interface_str[2];
		sscanf(line, "%s %s %s %s", prefix_str, next_hop_str, mask_str, interface_str);
		// fprintf(stderr, "PREFIX: %s NEXT_HOP: %s MASK: %s INTERFACE: %s\n",
										//  prefix_str, next_hop_str, mask_str, interface_str);
		rtable[i].prefix = inet_addr(prefix_str);
		rtable[i].next_hop = inet_addr(next_hop_str);
		rtable[i].mask = inet_addr(mask_str);
		rtable[i].interface = atoi(interface_str);
	}
    rtable_size = i;
	fclose(f);
    return rtable_size;
}



int binarySearch(int left, int right, uint32_t dest_ip) { 
    if (right >= left) { 
        int mid = left + (right - left) / 2; 
  
        if (rtable[mid].prefix == (rtable[mid].mask & dest_ip)) 
            return mid; 
  
        if (rtable[mid].prefix > (rtable[mid].mask & dest_ip)) 
            return binarySearch(left, mid - 1, dest_ip); 
  
        return binarySearch(mid + 1, right, dest_ip); 
    } 

    return -1; 
}

/*
 Returns a pointer (eg. &rtable[i]) to the best matching route
 for the given dest_ip. Or NULL if there is no matching route.
*/
struct route_table_entry *get_best_route(uint32_t dest_ip) {
	// rtable array is sorted by 2 criteria: prefix and mask
	// iterate untill find the last prefix equal to prefix from the index
	int index = binarySearch(0, rtable_size, dest_ip);

	if (index < 0){
		return NULL;
	} else {
		int i = index + 1;
		while (rtable[i].prefix == rtable[index].prefix){
			i++;
		}
		return &rtable[i - 1];
	}
}