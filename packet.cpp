#include "packet.h"

size_t create_packet(char* packet, unsigned int seq_num, size_t data_length, char* data) {
	// convert data into network type (big endian/little endian)
	unsigned int network_seq_num = htonl(seq_num);
	unsigned int network_data_length = htonl(data_length);

	// copy data into frame
	packet[0] = 0x1;
	memcpy(packet+1, &network_seq_num, 4);
	memcpy(packet+5, &network_data_length, 4);
	memcpy(packet+9, data, data_length);

	// TODO : needs to be implemented
	packet[9 + data_length] = 0x1; 

	return data_length + (size_t)10;
}

void read_packet(char* packet, unsigned int* seq_num, size_t* data_length, char* data, unsigned int* checksum) {
	// convert data
	unsigned int network_seq_num;
	unsigned int network_data_length;

	// byte copy
	memcpy(&network_seq_num, packet+1, 4);
	memcpy(&network_data_length, packet+5, 4);

	*seq_num = ntohl(network_seq_num);
	*data_length = ntohl(network_data_length);

	memcpy(data, packet+9, *data_length);
	
	// TODO : count the checksum
	*checksum = 0x1;
}

unsigned int count_checksum(size_t* data_length, char* data) {
	return (unsigned int)0;
}