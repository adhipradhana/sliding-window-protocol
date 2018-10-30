#ifndef ACK_H
#define ACK_H

#include <stdlib.h> 
#include <string.h>
#include <sys/types.h>
#include <netinet/in.h>
#include "packet.h"

#define ACK_LENGTH 6

void create_ack(char *ack, unsigned int seq_num, bool is_check_sum_valid);
void read_ack(char *ack, bool* is_nak, unsigned int *seq_num, bool *is_check_sum_valid);

#endif