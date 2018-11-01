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
    // Initialize socket variables
	int sock, socket_length, window_size;
    unsigned int port;
    socklen_t fromlen;
    struct sockaddr_in server;
    struct sockaddr_in from;

    // Initialize sliding window variables
    int laf, lfr;
    int buffer_size, max_buffer_size;
    char *buffer;
    char *fname;
    FILE *file;
    bool eot;

    // Initialize packet variables
    unsigned int seq_num;
    bool is_check_sum_valid;
    size_t data_length;
    char ack[ACK_LENGTH];
    char packet[MAX_PACKET_LENGTH];
    char data[MAX_DATA_LENGTH];

    // Read data
    if (argc != 5) {
    	cerr << "usage: ./recvfile <filename> <window_size> <buffer_size> <port>" << endl;
    	exit(1);
    }
    fname = argv[1];
    window_size = atoi(argv[2]);
    max_buffer_size = (unsigned int)1024 * atoi(argv[3]);
    port = atoi(argv[4]);

    // Create socket
    sock = socket(AF_INET, SOCK_DGRAM, 0);
    if (sock < 0) {
    	cerr << "Error creating socket" << endl;
    	exit(1);
    }

    // Assign server port and address
    socket_length = sizeof(server);
    bzero(&server, socket_length);

    server.sin_family = AF_INET;
    server.sin_addr.s_addr = INADDR_ANY;
    server.sin_port = htons(port);

    // Bind socket
    if (bind(sock, (struct sockaddr *) &server, socket_length) < 0) {
        cerr << "Can't bind to address" << endl;
        exit(1);
    }

    // Write to file
    file = fopen(fname, "wb");

    bool recv_done = false;
    while (!recv_done) {
        buffer = new char[max_buffer_size];
        buffer_size = max_buffer_size;

        int seq_count = max_buffer_size / MAX_DATA_LENGTH;
        bool packet_received[window_size];
        for (int i = 0; i < window_size; i++) {
            packet_received[i] = false;
        }

        lfr = -1;
        laf = lfr + window_size;

        fromlen = sizeof(struct sockaddr_in);
        while (true) {
        	// Block receiving message
            int packet_size = recvfrom(sock, packet, MAX_PACKET_LENGTH, MSG_WAITALL, (struct sockaddr *)&from, &fromlen);
            if (packet_size < 0) {
                cout << "Error on receiving message\n";
                // exit(1);
            }

            // Get packet
            read_packet(packet, &seq_num, &data_length, data, &is_check_sum_valid, &eot);

            // Create ack
            create_ack(ack, seq_num, is_check_sum_valid);

            // Send ack
            int ack_size = sendto(sock, ack, ACK_LENGTH, MSG_WAITALL, (struct sockaddr *)&from, fromlen);
            if (ack_size < 0) {
                cout << "Fail sending ack\n";
            }

            if (seq_num <= laf) {
                
                if (is_check_sum_valid) {
                    int buffer_shift = seq_num * MAX_DATA_LENGTH;

                    if (seq_num == lfr + 1) {
                        memcpy(buffer + buffer_shift, data, data_length);
                        unsigned int shift = 1;
                        for (unsigned int i = 1; i < window_size; i++) {
                            if (!packet_received[i]) {
                                break;
                            }
                            shift++;
                        }
                        cout << "shift packet_received\n";
                        for (unsigned int i = 0; i < window_size - shift; i++) {
                            packet_received[i] = packet_received[i + shift];
                        }
                        cout << "false-in packet_received\n";
                        for (unsigned int i = window_size - shift; i < window_size; i++) {
                            packet_received[i] = false;
                        }
                        lfr += shift;
                        laf = lfr + window_size;
                    } else if (seq_num > lfr + 1) {
                        if (!packet_received[seq_num - (lfr + 1)]) {
                            memcpy(buffer + buffer_shift, data, data_length);
                            packet_received[seq_num - (lfr + 1)] = true;

                        }
                    }

                    if (eot) {
                        buffer_size = buffer_shift + data_length;
                        seq_count = seq_num + 1;
                        recv_done = true;
                        cout << "Receive packet eot " << seq_num << endl;
                        cout << "Sending ack eot " << seq_num << endl;
                    } else {
                        cout << "Receive packet  " << seq_num << endl;
                        cout << "Sending ack  " << seq_num << endl;
                    }
                } else {
                    cout << "ERROR packet " << seq_num << endl;
                    cout << "Sending NAK : " << seq_num << endl;
                }
            } else {
                // Send negative ack
                cout << "lfr : " << lfr << " laf : " << laf << "\n";
                cout << "SeqNum out of range : " << seq_num << endl;
            }

            if (lfr >= seq_count - 1) {
                break;
            }
        }

        fwrite(buffer, 1, buffer_size, file);
    }

    fclose(file);
    return 0;
}