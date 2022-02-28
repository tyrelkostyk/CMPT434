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
#include <sys/types.h>
#include <sys/socket.h>

#include "defs.h"



int main(int argc, char* argv[])
{
	printf("Client initializing...\n");

	// obtain connection settings from program arguments
	char* hostname = argv[1];
	char* port = argv[2];

	int error = 0;						// error tracking
	int send_socket = 0;				// socket to transmit on
	struct addrinfo hints, *info, *p;	// obtaining socket address info
	int flags = 0;						// connection flagsc

	size_t max_buffer_size = MAX_BUFFER_LENGTH;	// max buffer size
	char *input_message = NULL;					// local input message (from stdin)
	int input_message_length = 0;				// length of input message
	int bytes_sent, bytes_sent_tmp = 0;			// bytes sent by sendto() call

	char recv_buffer[MAX_BUFFER_LENGTH];	// local receive buffer
   	char send_buffer[MAX_BUFFER_LENGTH];	// local transmit buffer

	// prepare for a UDP socket using IPv4 on the local machine
   	memset(&hints, 0, sizeof(hints));
   	hints.ai_family = AF_INET;		// use IPv4
   	hints.ai_socktype = SOCK_DGRAM;	// UDP socket

	// obtain information about the location of available UDP sockets
	error = getaddrinfo(hostname, port, &hints, &info);
	if (error != 0) {
		fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(error));
		return 2;
	}

	// loop through all the results and bind to the first we can
	for (p = info; p != NULL; p = p->ai_next) {
		send_socket = socket(p->ai_family, p->ai_socktype, p->ai_protocol);
		if (send_socket == -1) {
			perror("client: socket");
			continue;
		}

		// a socket was successfully bound! All done here
		break;
	}

	// ensure a socket was bound to
	if (p == NULL) {
		fprintf(stderr, "client: failed to bind\n");
		return 3;
	}

	// all done with this structure; free it
	freeaddrinfo(info);

	// begin talking
	debug(("Client: Ready to transmit...\n"));

	while (1) {

		// reset counters, buffers
		bytes_sent = 0;
		bytes_sent_tmp = 0;
		input_message_length = 0;
		input_message = NULL;
		memset(recv_buffer, 0, sizeof(recv_buffer));
		memset(send_buffer, 0, sizeof(send_buffer));

		// obtain user input message from stdin
		fputs("send>> ", stdout);
        getline(&input_message, &max_buffer_size, stdin);

		// Strip newline
        input_message_length = strlen(input_message);
        input_message[input_message_length-1] = 0;

		// TODO: add sequence number, place in packet struct

		// TODO: add packet to FIFO queue

		// send the message
		do {
			// send over UDP
			bytes_sent_tmp = sendto(send_socket,
									&input_message[bytes_sent],
									input_message_length,
									flags, NULL, 0);
			// update how many bytes remaining
			bytes_sent += bytes_sent_tmp;
			input_message_length -= bytes_sent_tmp;
		} while (input_message_length > 0);  // account for truncation during send

		// TODO: receive ACK

	}

	return -1;
}
