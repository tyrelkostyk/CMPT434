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

static void packetHandle(const packet_t* packet);
static void packetSend(void);
static void packetReceive(char* recv_buffer);


/*******************************************************************************
                         PRIVATE DEFINITIONS AND GLOBALS
*******************************************************************************/

int receiver_socket = 0;	// socket for listening
int flags = 0;				// no flags necessary for receiving/sending

struct sockaddr_storage sender_addr = { 0 };		// address of the sender
socklen_t sender_addr_size = sizeof(sender_addr);	// size of sender's address

int expected_sequence_number = 0;


/*******************************************************************************
                                      MAIN
*******************************************************************************/

int main(int argc, char* argv[])
{
	// confirm number of input arguments
   if (argc != 2) {
		printf("Error: invalid input received\n");
		exit(-1);
	}

	// obtain settings from input arguments
	char* recv_port = argv[1];

	// confirm port number is within valid bounds
	if (atoi(recv_port) < PORT_MIN || atoi(recv_port) > PORT_MAX) {
		printf("Error: port number not valid\n");
		exit(-1);
	}

	printf("UDP Receiver initializing...\n");
	printf("Sender may connect to the UDP Receiver via Address %s on Port %s\n",
		   LOCAL_ADDRESS, recv_port);

	int error;								// error checking
   	struct addrinfo hints, *info, *p;		// obtaining socket address info
	char recv_buffer[MAX_BUFFER_LENGTH] = { 0 };	// local receive buffer

	// BIND TO A UDP SOCKET

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
   		receiver_socket = socket(p->ai_family, p->ai_socktype, p->ai_protocol);
   		if (receiver_socket == -1) {
   			perror("Receiver: socket");
   			continue;
   		}

   		if (bind(receiver_socket, p->ai_addr, p->ai_addrlen) == -1) {
   			close(receiver_socket);
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

	// BEGIN RECEIVING

	debug(("Receiver: Listening...\n"));

	while (1) {

		// reset receive buffer
		memset(recv_buffer, 0, sizeof(recv_buffer));

		// receive message from sender
		packetReceive(recv_buffer);

		// handle received packet
		packetHandle((const packet_t *)recv_buffer);

		// exract the received sequence number
		int received_sequence_number = ((packet_t*)recv_buffer)->sequence_number;

		// send ACK and simulate dropped packets if received packet is correct (or a retransmission)
		if ((received_sequence_number == expected_sequence_number)
		||  (received_sequence_number == expected_sequence_number-1))
		{
			// send ACK packet
			packetSend(expected_sequence_number);

			// TODO: add simulation for corrupted messages and responses

		}

		// increment expected sequence number if this one was expected
		if (received_sequence_number == expected_sequence_number)
			expected_sequence_number++;
	}
}


/*******************************************************************************
                               PRIVATE FUNCTIONS
*******************************************************************************/

static void packetHandle(const packet_t* packet)
{
	// confirm that packet pointer is valid
	if (packet == 0) {
		debug(("packetHandle: input pointer is NULL\n"));
		exit(-1);
	}

	// print the received message and sequence number to stdout
	printf("%d: %s\n", packet->sequence_number, packet->message);
}


static void packetSend(int expected)
{
	// populate ACK packet with next expected sequence number
	packet_t ack_packet = { 0 };
	ack_packet.sequence_number = expected;
	if (ack_packet.sequence_number == SEQUENCE_NUMBER_MAX)
		ack_packet.sequence_number = 0;

	// ACK packet only populates the header; no message
	int send_size = HEADER_SIZE;

	// send the ACK packet
	do {
		debug(("packetSend: about to send: %d -- Len: %d\n", ack_packet.sequence_number, send_size));

		// send over UDP
		int bytes_sent = 0;
		int bytes_sent_tmp = 0;
		bytes_sent_tmp = sendto(receiver_socket,
								&((char*)&ack_packet)[bytes_sent],
								send_size,
								flags,
								(const struct sockaddr *)&sender_addr,
								sender_addr_size);

		// check for errors
		if (bytes_sent_tmp <= 0) {
			printf("packetSend: failed to send (%d)\n", bytes_sent_tmp);
			exit(-1);
		}

		// increment counter
		bytes_sent += bytes_sent_tmp;
		send_size -= bytes_sent;
	} while (send_size > 0);  // account for truncation during send
}


static void packetReceive(char* recv_buffer)
{
	// receive message from client
	int bytes_received = recvfrom(receiver_socket,
								  recv_buffer,
								  MAX_BUFFER_LENGTH-1,
								  flags,
								  (struct sockaddr *)&sender_addr,
								  &sender_addr_size);

	if (bytes_received <= 0) {
		printf("Receiver: failed to receive\n");
		exit(-1);
	}

	debug(("%d bytes received from sender\n", bytes_received));
}
