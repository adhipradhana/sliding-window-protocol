all: recvfile sendfile

recvfile: src/packet.h src/packet.cpp src/ack.h src/ack.cpp src/recvfile.cpp
	g++ -pthread src/packet.cpp src/ack.cpp src/recvfile.cpp -o recvfile -std=gnu++17

sendfile: src/packet.h src/packet.cpp src/ack.h src/ack.cpp src/sendfile.cpp
	g++ -pthread src/packet.cpp src/ack.cpp src/sendfile.cpp -o sendfile -std=gnu++17
