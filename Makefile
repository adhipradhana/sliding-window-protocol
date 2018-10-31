all: recvfile sendfile

recvfile: packet.h packet.cpp ack.h ack.cpp recvfile.cpp
	g++ -pthread packet.cpp ack.cpp recvfile.cpp -o recvfile -std=c++11 

sendfile:packet.h packet.cpp ack.h ack.cpp sendfile.cpp
	g++ -pthread packet.cpp ack.cpp sendfile.cpp -o sendfile -std=c++11 

clean: recvfile sendfile
	rm -f recvfile sendfile
