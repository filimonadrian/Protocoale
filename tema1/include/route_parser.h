#pragma once
#include <stdio.h>
#include <unistd.h>
#include <stdint.h>
#include <string.h>
#include <stdlib.h>
#include <arpa/inet.h>

struct route_table_entry {
	uint32_t prefix;
	uint32_t next_hop;
	uint32_t mask;
	int interface;
} __attribute__((packed));

struct route_table_entry *rtable;
int rtable_size;

int parse_route_table();
int compare_func(const void *a, const void *b);
int binarySearch(int left, int right, uint32_t dest_ip);
struct route_table_entry *get_best_route(uint32_t dest_ip);

