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
#include "ack.h"
#include "timer.h"

using namespace std;

// global variable
int sock, window_size, buffer_size;
unsigned int sock_length;
struct sockaddr_in server, from;
struct hostent *hp;

unsigned int lar, lfs;
bool *has_ack_received;
bool *has_packet_send;
time_stamp *packet_send_time;

time_stamp TMIN = current_time();

void get_ack() {
    char ack[ACK_LENGTH];
    unsigned int seq_num;
    bool is_check_sum_valid, is_nak;


    while(true) {     
        int ack_size = recvfrom(sock, ack, ACK_LENGTH, MSG_WAITALL,(struct sockaddr *)&from, &sock_length);
        if (ack_size < 0) {
            cout << "Packet loss on sending message" << endl;
            exit(1);
        }

        // create ack from buffer
        read_ack(ack, &is_nak, &seq_num, &is_check_sum_valid);

        if (is_check_sum_valid) {
            if (seq_num >= lar + 1 && seq_num <= lfs) {
                if (!is_nak) {
                    has_ack_received[seq_num - (lar + 1)] = true;
                    cout << "Receiving ACK : " << seq_num << endl;
                } else {
                    packet_send_time[seq_num - (lar + 1)] = TMIN;
                    cout << "Receiving NAK : " << seq_num << endl;
                }
            } else {
                cout << "ACK out of bound " << seq_num << endl;
            }
        } else {
            cout << "ERROR ACK : " << seq_num << endl;
        }
    }
}

int main(int argc, char *argv[]) {
    // packet related data
    unsigned int packet_size, data_size;
    char data[MAX_DATA_LENGTH];
    char packet[MAX_PACKET_LENGTH];

    if (argc != 5) {
    	cerr << "usage: ./sendfile <window_size> <buffer_size> <destination_ip> <destination_port>" << endl;
    	exit(1);
    }

    // get data from argument
    window_size = atoi(argv[1]);
    buffer_size = atoi(argv[2]);

    // creating socket
    sock = socket(AF_INET, SOCK_DGRAM, 0);
    if (sock < 0) {
    	cerr << "ERROR on creating socket" << endl;
    	exit(1);
    }

    // get host name
    server.sin_family = AF_INET;
    hp = gethostbyname(argv[3]);
    if (hp == 0) {
    	cerr << "ERROR on getting host name" << endl;
    	exit(1);
    }

    // fill server data struct
    bcopy((char *)hp->h_addr, (char *)&server.sin_addr, hp->h_length);
    server.sin_port = htons(atoi(argv[4]));
    sock_length = sizeof(struct sockaddr_in);

    // Create a receiver thread
    thread receiver_thread(get_ack);

    // Initialized sliding window variable
    lar = -1;
    lfs = lar + window_size;
    has_ack_received = new bool[window_size];
    packet_send_time = new time_stamp[window_size];
    bool has_packet_send[window_size];
    unsigned int seq_num;
    unsigned int seq_count = 10; // TODO FROM BUFFER
    for (int i = 0; i < window_size; i++) {
        has_ack_received[i] = false;
        has_packet_send[i] = false;
    }

    while (true) {
        // check shift
        if (has_ack_received[0]) {
            int shift = 0;

            for (unsigned int i = 1; i < window_size; i++) {
                if (!has_ack_received[i]) {
                    break;
                }
                shift++;
            }

            for (unsigned int i = 0; i < window_size - shift; i++) {
                has_packet_send[i] = has_packet_send[i + shift];
                has_ack_received[i] = has_ack_received[i + shift];
                packet_send_time[i] = packet_send_time[i + shift];
            }

            for (unsigned int i = window_size - shift; i < window_size; i++) {
                has_packet_send[i] = false;
                has_ack_received[i] = false;
            }

            lar += shift;
            lfs = lar + window_size;
        }

        for (int i = 0; i < window_size; i ++) {
            seq_num = lar + i + 1;
            if (seq_num < seq_count) {
                if (!has_packet_send[i] || ((!has_ack_received[i]) && (elapsed_time(current_time(), packet_send_time[i]) > TIMEOUT))) {
                    // unsigned int buffer_shift = seq_num * MAX_DATA_LENGTH;
                    // data_size = (buffer_size - buffer_shift < MAX_DATA_LENGTH) ? (buffer_size - buffer_shift) : MAX_DATA_LENGTH;

                    strcpy(data, "test message");
                    data_size = strlen(data);
                    
                    bool eot = (seq_num == seq_count - 1); //&& (read_done);
                    packet_size = create_packet(packet, seq_num, data_size, data);

                    int packet_size = sendto(sock, packet, packet_size, MSG_WAITALL, (const struct sockaddr *) &server, sock_length);
                    if (packet_size < 0) {
                        cerr << "ERROR sending packet\n";
                        exit(1);
                    }

                    has_packet_send[i] = true;
                    packet_send_time[i] = current_time();

                    cout << "Sending package " << seq_num << endl;
                    // if (!eot) {
                    //     cout << "[SENT FRAME " << seq_num << "] " << data_size << " bytes" << endl;
                    // } else {
                    //     cout << "[SENT EOT FRAME " << seq_num << "] " << data_size << " bytes" << endl;
                    // }
                }
            }
        }

        if (seq_num == 10) break;
    }

    receiver_thread.join();
    close(sock);

	return 0;
}