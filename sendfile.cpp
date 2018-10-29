#include <iostream>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <thread>

#include "packet.h"

using namespace std;

// global variable
int sock, window_size, buffer_size;
unsigned int length;
struct sockaddr_in server, from;
struct hostent *hp;
unsigned int lar, lfs;

void get_ack() {
    char buffer[256];

    while(1) {
        int ack_size = recvfrom(sock, buffer, 256, 0,(struct sockaddr *)&from, &length);
        if (ack_size < 0) {
            cout << "Packet loss on sending message" << endl;
            exit(1);
        }
        lar++;
        cout << "LAR : " << lar << endl;
    }
}

int main(int argc, char *argv[]) {
    // packet related data
    unsigned int len_packet;
    char data[MAX_DATA_LENGTH];
    char packet[MAX_PACKET_LENGTH];

    if (argc != 5) {
    	cerr << "usage: ./sendfile <window_size> <buffer_size> <destination_ip> <destination_port>" << endl;
    	return -1;
    }

    // get data from argument
    window_size = atoi(argv[1]);
    buffer_size = atoi(argv[2]);

    // creating socket
    sock = socket(AF_INET, SOCK_DGRAM, 0);
    if (sock < 0) {
    	cerr << "Error on creating socket" << endl;
    	return -1;
    }

    // get host name
    server.sin_family = AF_INET;
    hp = gethostbyname(argv[3]);
    if (hp == 0) {
    	cerr << "Error on getting host name" << endl;
    	return -1;
    }

    // fill server data struct
    bcopy((char *)hp->h_addr, (char *)&server.sin_addr, hp->h_length);
    server.sin_port = htons(atoi(argv[4]));
    length = sizeof(struct sockaddr_in);

    // set last acknowledgement received
    lar = 0;
    lfs = 0;

    // create thread
    thread receiver_thread(get_ack);

    while (1) {
        if (lfs - lar < window_size) {
            strcpy(data, "memek kontol");

            // create a packet
            len_packet = create_packet(packet, (unsigned int)lfs, (size_t)13, data);

            // sending packet
            int packet_size = sendto(sock, packet, len_packet, 0,(const struct sockaddr *) &server,length);
            if (packet_size < 0) {
                cout << "Error on sending message" << endl;
                return -1;
            }
            lfs++;
        }
    }

    // close thread
    receiver_thread.join();
    // close the socket
    close(sock);

	return 0;
}