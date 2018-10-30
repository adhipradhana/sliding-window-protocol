#include <iostream>
#include <sys/types.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <string.h>
#include <netdb.h>

#include "packet.h"
#include "ack.h"

using namespace std;

int main(int argc, char *argv[]) {
	int sock, length, n, window_size, buffer_size;
    socklen_t fromlen;
    struct sockaddr_in server;
    struct sockaddr_in from;

    // sliding window
    unsigned int laf, lfr;

    // packet description
    unsigned int seq_num;
    bool is_check_sum_valid;
    size_t data_length;
    char ack[ACK_LENGTH];
    char packet[MAX_PACKET_LENGTH];
    char data[MAX_DATA_LENGTH];
    bool* packet_received;


    // read data
    if (argc != 4) {
    	cerr << "usage: ./recvfile <window_size> <buffer_size> <port>" << endl;
    	return -1;
    }
    window_size = atoi(argv[1]);
    buffer_size = atoi(argv[2]);

    // create socket
    sock = socket(AF_INET, SOCK_DGRAM, 0);
    if (sock < 0) {
    	cerr << "Error creating socket" << endl;
    	return -1;
    }

    // assigning server port and address
    length = sizeof(server);
    bzero(&server, length);

    server.sin_family = AF_INET;
    server.sin_addr.s_addr = INADDR_ANY;
    server.sin_port = htons(atoi(argv[3]));

    if (bind(sock, (struct sockaddr *) &server,length) < 0) {
        cerr << "Can't bind to address" << endl;
        return -1;
    }

    // sliding window
    lfr = -1;
    laf = lfr + window_size;
    packet_received = new bool[window_size];

    fromlen = sizeof(struct sockaddr_in);
    while (true) {
    	// blocking receiving message
        n = recvfrom(sock, packet, MAX_PACKET_LENGTH, 0, (struct sockaddr *)&from, &fromlen);
        if (n < 0) {
            cout << "Error on receiving message\n";
            exit(1);
        }

        // get packet
        read_packet(packet, &seq_num, &data_length, data, &is_check_sum_valid);

        if (seq_num <= laf) {
            cout << "SeqNum : " << seq_num << " Data Length : " << data_length << " Message : " << data << endl;

            // create ack
            create_ack(ack, seq_num, is_check_sum_valid);

            // sending ack
            n = sendto(sock, ack, ACK_LENGTH, 0, (struct sockaddr *)&from, fromlen);
            if (n  < 0) {
                cout << "Fail sending ack\n";
            }

            if (is_check_sum_valid) {
                int shift = 0;

                if (seq_num == lfr +1) {
                    for (int i = 0; i < window_size; i++) {
                        shift++;
                        if (!packet_received[i]) {
                            break;
                        }
                    }

                    for (int i = 0; i < window_size - shift; i++) {
                        packet_received[i] = packet_received[i + shift];
                    }

                    for (int i = window_size - shift; i < window_size; i++) {
                        packet_received[i] = false;
                    }

                    lfr += shift;
                    laf = lfr + window_size;
                } else if (seq_num > lfr + 1) {
                    packet_received[seq_num - (lfr + 1)] = true;
                }
                cout << "Sending ACK : " << seq_num << endl;
            } else {
                cout << "Sending NAK : " << seq_num << endl;
            }
        } else {
            // sending negative ack
            cout << "SeqNum out of range : " << seq_num << endl;
        }
    }


    return 0;
}