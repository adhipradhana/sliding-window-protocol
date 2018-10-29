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

	packet[9 + data_length] = count_checksum(data_length, data);

	return data_length + (size_t)10;
}

void read_packet(char* packet, unsigned int* seq_num, size_t* data_length, char* data, bool* is_check_sum_valid) {
	// convert data
	unsigned int network_seq_num;
	unsigned int network_data_length;

	// byte copy
	memcpy(&network_seq_num, packet+1, 4);
	memcpy(&network_data_length, packet+5, 4);

	*seq_num = ntohl(network_seq_num);
	*data_length = ntohl(network_data_length);

	memcpy(data, packet+9, *data_length);

	char sender_check_sum = packet[9 + *data_length];
	char checksum = count_checksum(*data_length, data);

	*is_check_sum_valid = sender_check_sum == checksum;
}

char count_checksum(size_t data_length, char* data) {
	unsigned long sum = 0;
	for (size_t i = 0; i < data_length; i++) {
		sum += *data++;
		if (sum & 0xFF00) {
			sum &= 0xFF;
			sum++;
		}
	}
	return (char)(~(sum & 0xFF));
}