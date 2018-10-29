#include <iostream>
#include <sys/types.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <string.h>
#include <netdb.h>

#include "packet.h"

using namespace std;

int main(int argc, char *argv[]) {
	int sock, length, n, window_size, buffer_size;
    socklen_t fromlen;
    struct sockaddr_in server;
    struct sockaddr_in from;

    // packet description
    unsigned int seq_num;
    unsigned int checksum;
    size_t data_length;
    char packet[MAX_PACKET_LENGTH];
    char data[MAX_DATA_LENGTH];

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

    fromlen = sizeof(struct sockaddr_in);
    while (1) {
    	// blocking receiving message
        n = recvfrom(sock, packet, MAX_PACKET_LENGTH, 0, (struct sockaddr *)&from, &fromlen);
        if (n < 0) {
            cout << "Error on receiving message\n";
        }

        // get packet
        read_packet(packet, &seq_num, &data_length, data, &checksum);

        cout << "SeqNum : " << seq_num << " Data Length : " << data_length << " Message : " << data << endl;

        // sending ack
        n = sendto(sock, "Got your message\n", 17, 0, (struct sockaddr *)&from, fromlen);
        if (n  < 0) {
            cout << "Error on sending message\n";
        }
    }


    return 0;
}