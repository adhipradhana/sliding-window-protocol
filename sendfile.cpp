#include <iostream>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>

#include "packet.h"

using namespace std;

int main(int argc, char *argv[]) {
	int sock, n, window_size, buffer_size;
    unsigned int length;
    struct sockaddr_in server, from;
    struct hostent *hp;
    char buffer[256];

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

    // get data
    cout << "Please enter the message : ";
    cin >> data;

    // create a packet
    len_packet = create_packet(packet, (unsigned int)1, (size_t)MAX_DATA_LENGTH, data);

    // sending packet
    n = sendto(sock, packet, len_packet, 0,(const struct sockaddr *) &server,length);
    if (n < 0) {
        cout << "Error on sending message" << endl;
        return -1;
    }

    // receiving ack
    n = recvfrom(sock, buffer, 256, 0,(struct sockaddr *)&from, &length);
    if (n < 0) {
        cout << "Packet loss on sending message" << endl;
        return -1;
    }

   	cout << "Got an ack : " << buffer << endl;

    // close the socket
    close(sock);

	return 0;
}