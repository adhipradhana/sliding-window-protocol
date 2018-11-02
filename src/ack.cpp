#include "ack.h"

void create_ack(char *ack, unsigned int seq_num, bool is_check_sum_valid){
	
	if(is_check_sum_valid)
		ack[0] = 0x0;
	else
		ack[0] = 0x1;

	uint32_t net_seq_num = htonl(seq_num);
	memcpy(ack + 1, &net_seq_num, 4);
	ack[5] = count_checksum((size_t)4, (char *) &seq_num);
}

void read_ack(char *ack, bool* is_nak, unsigned int *seq_num, bool *is_check_sum_valid){
	
	if(ack[0] == 0x0)
		*is_nak = false;
	else
		*is_nak = true;

	uint32_t net_seq_num;
	memcpy(&net_seq_num, ack + 1, 4);
	*seq_num = ntohl(net_seq_num);

	*is_check_sum_valid = ack[5] == count_checksum((size_t)4, (char *) seq_num);
}