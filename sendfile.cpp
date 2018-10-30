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
int sock, window_size;
unsigned int sock_length, port;
struct sockaddr_in server, from;
struct hostent *hp;
char *ip;

int lar, lfs;
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
            cout << "Packet loss on receiving message" << endl;
            exit(1);
        }
        // create ack from buffer
        read_ack(ack, &is_nak, &seq_num, &is_check_sum_valid);

        if (is_check_sum_valid) {
            if (seq_num >= lar + 1 && seq_num <= lfs) {
                if (!is_nak) {
                    cout << "=========== ACK REPORT ============\n";
                    cout << "- seq_num        : " << seq_num << "\n";
                    cout << "- lar            : " << lar << "\n";
                    cout << "- modified_index : " << seq_num - (lar + 1) << "\n";
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
    unsigned int seq_num;
    unsigned int packet_size, data_size;
    char data[MAX_DATA_LENGTH];
    char packet[MAX_PACKET_LENGTH];

    // file related variable
    FILE *file;
    char *buffer;
    char *filename;
    unsigned int max_buffer_size, buffer_size;


    if (argc != 6) {
    	cerr << "usage: ./sendfile <filename> <window_size> <buffer_size> <destination_ip> <destination_port>" << endl;
    	exit(1);
    }

    // get data from argument
    filename = argv[1];
    window_size = atoi(argv[2]);
    max_buffer_size = atoi(argv[3]) * (unsigned int) MAX_DATA_LENGTH;
    ip = argv[4];
    port = atoi(argv[5]);

    // creating socket
    sock = socket(AF_INET, SOCK_DGRAM, 0);
    if (sock < 0) {
    	cerr << "ERROR on creating socket" << endl;
    	exit(1);
    }

    // get host name
    server.sin_family = AF_INET;
    hp = gethostbyname(ip);
    if (hp == 0) {
    	cerr << "ERROR on getting host name" << endl;
    	exit(1);
    }

    // fill server data struct
    bcopy((char *)hp->h_addr, (char *)&server.sin_addr, hp->h_length);
    server.sin_port = htons(port);
    sock_length = sizeof(struct sockaddr_in);

    // Open file
    if (access(filename, F_OK) < 0) {
        cerr << "FILE not exist\n";
        exit(1);
    } 
    file = fopen(filename, "rb");
    buffer = new char[max_buffer_size];


    // Create a receiver thread
    thread receiver_thread(get_ack);

    bool read_done = false;
    while (!read_done) {

        // read file from buffer
        buffer_size = fread(buffer, 1, max_buffer_size, file);
        read_done = max_buffer_size > buffer_size;

        // Initialized sliding window variable
        has_ack_received = new bool[window_size];
        packet_send_time = new time_stamp[window_size];
        bool has_packet_send[window_size];
        unsigned int seq_count = buffer_size / MAX_DATA_LENGTH;
        if (buffer_size % MAX_DATA_LENGTH) {
            seq_count++;
        }
        cout << "seq_count : " << seq_count << "\n";

        // set to false
        for (int i = 0; i < window_size; i++) {
            has_packet_send[i] = false;
            has_ack_received[i] = false;
        }

        lar = -1;
        cout << "setting lar into -1" << endl;
        lfs = lar + window_size;

        bool send_done = false;
        while (!send_done) {
            // check shift
            if (has_ack_received[0]) {
                unsigned int shift = 1;

                for (unsigned int i = 1; i < window_size; i++) {
                    if (!has_ack_received[i]) {
                        break;
                    }
                    shift++;
                }
                cout << "shift : " << shift << endl;

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
                cout <<"lar " << lar << endl;
            }

            for (unsigned int i = 0; i < window_size; i ++) {
                seq_num = lar + i + 1;

                if (seq_num < seq_count) {
                    if (!has_packet_send[i] || ((!has_ack_received[i]) && (elapsed_time(current_time(), packet_send_time[i]) > TIMEOUT))) {
                        unsigned int buffer_shift = seq_num * MAX_DATA_LENGTH;
                        data_size = (buffer_size - buffer_shift < MAX_DATA_LENGTH) ? (buffer_size - buffer_shift) : MAX_DATA_LENGTH;
                        memcpy(data, buffer + buffer_shift, data_size);
                        
                        bool eot = (seq_num == seq_count - 1) && (read_done);
                        packet_size = create_packet(packet, seq_num, data_size, data, eot);

                        int n = sendto(sock, packet, packet_size, MSG_WAITALL, (const struct sockaddr *) &server, sock_length);
                        if (n < 0) {
                            cerr << "ERROR sending packet\n";
                            exit(1);
                        }

                        has_packet_send[i] = true;
                        packet_send_time[i] = current_time();

                        if (!eot) {
                            cout << "Sending package " << seq_num << endl;
                        } else {
                            cout << "Sending eot package " << seq_num << endl;
                        }
                    }
                }
            }

            cout << "lar : " << lar << "\n";
            cout << "seq_count : " << seq_count << "\n";
            if (lar >= seq_count - 1) {
                send_done = true;
            }

            if (seq_count < window_size) {
                send_done = true;
                // cout << "=---------=-==-=-=-=-=-=-\n";
                for (unsigned int i = 0; i < seq_count; i++) {
                    send_done &= has_ack_received[i];
                    // if (!has_ack_received[i]) {
                    //     send_done = false;
                    //     break;
                    // }
                }
                // cout << "\nsend done jadinya " << send_done << "\n";
            }    
        }
        cout << "is read done ? " << read_done << "\n";
        if (read_done) {
            break;
        }
    }

    fclose(file);
    receiver_thread.detach();
    // close(sock);

	return 0;
}