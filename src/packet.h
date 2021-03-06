#ifndef PACKET_H
#define PACKET_H

#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <netinet/in.h>

#include <iostream>
using namespace std;


#define MAX_DATA_LENGTH 1024
#define MAX_PACKET_LENGTH 1034

size_t create_packet(char* packet, unsigned int seq_num, size_t data_length, char* data, bool eot);

void read_packet(char* packet, unsigned int* seq_num, size_t* data_length, char* data, bool* is_check_sum_valid, bool* eot);

char count_checksum(size_t data_length, char* data);

#endif