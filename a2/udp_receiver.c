/**
 * CMPT 434 Assignment 2
 * Monday February 28 2022
 * Tyrel Kostyk - tck290 - 11216033
**/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <netdb.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>

#include "defs.h"


/*******************************************************************************
                             PRIVATE FUNCTION STUBS
*******************************************************************************/



/*******************************************************************************
                                      MAIN
*******************************************************************************/

int main(int argc, char* argv[])
{
   if (argc < 2) {
		printf("Error: invalid input received");
		exit(-1);
	}

	char* recv_port = argv[1];

	printf("UDP Receiver initializing...\n");
	printf("Sender may connect to the UDP Receiver via Address %s on Port %s\n",
		   LOCAL_ADDRESS, recv_port);

	int error;								// error checking
	int listen_socket;						// socket for listening
   	struct addrinfo hints, *info, *p;		// obtaining socket address info
	int flags = 0;							// no flags necessary for receiving/sending

   	// struct sockaddr_storage their_addr;	// the new connection's address
   	// socklen_t addr_size;					// size of incoming socket address
   	// socket_info_t socket_addr = { 0 };	// info and addr of new socket

   	int bytes_received = 0;					// bytes read by recvfrom() call

   	char recv_buffer[MAX_BUFFER_LENGTH];	// local receive buffer
   	// char send_buffer[MAX_BUFFER_LENGTH];	// local transmit buffer

	packet_t packet = { 0 };
	int sequence_number = 0;

	// prepare for a UDP socket using IPv4 on the local machine
   	memset(&hints, 0, sizeof(hints));
   	hints.ai_family = AF_INET;		// use IPv4
   	hints.ai_socktype = SOCK_DGRAM;	// UDP socket
	hints.ai_flags = AI_PASSIVE;	// use host machine IP

   	// obtain information about the location of available UDP sockets
   	error = getaddrinfo(NULL, recv_port, &hints, &info);
   	if (error) {
   		fprintf(stderr, "Receiver: getaddrinfo: %s\n", gai_strerror(error));
   		exit(-1);
   	}

   	// loop through all the results and bind to the first we can
   	for (p = info; p != NULL; p = p->ai_next) {
   		listen_socket = socket(p->ai_family, p->ai_socktype, p->ai_protocol);
   		if (listen_socket == -1) {
   			perror("Receiver: socket");
   			continue;
   		}

   		if (bind(listen_socket, p->ai_addr, p->ai_addrlen) == -1) {
   			close(listen_socket);
   			perror("Receiver: bind");
   			continue;
   		}

   		// a socket was successfully bound! All done here
   		break;
   	}

   	// ensure a socket was bound to
   	if (p == NULL) {
   		fprintf(stderr, "Receiver: failed to bind\n");
   		exit(-1);
   	}

	// all done with this structure; free it
   	freeaddrinfo(info);

	// begin listening
	debug(("Receiver: Listening...\n"));

	while (1) {

		// reset counters, buffers
		memset(recv_buffer, 0, sizeof(recv_buffer));
		memset(&packet, 0, sizeof(packet));
		bytes_received = 0;

		// receive message from client
		bytes_received = recvfrom(listen_socket, recv_buffer, MAX_BUFFER_LENGTH-1, flags, NULL, NULL);

		if (bytes_received <= 0) {
			printf("Receiver: failed to receive\n");
			exit(-1);
		}

		debug(("%d bytes received from sender\n", bytes_received));

		// convert received data into packet
		memcpy(&packet, recv_buffer, bytes_received);

		// print the received message and sequence number to stdout
		printf("%d: %s\n", packet.sequence_number, packet.message);

		// TODO: check for validity

		// TODO: add simulation for corrupted messages and responses

		// TODO: send ACK

	}

	return -1;
}
