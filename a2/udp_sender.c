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


/*******************************************************************************
                                      MAIN
*******************************************************************************/

int main(int argc, char* argv[])
{
	if (argc < 5) {
		printf("Error: invalid input received");
		exit(-1);
	}

	// obtain settings from program arguments
	char* hostname = argv[1];
	char *port = argv[2];
	int window_size = atoi(argv[3]);
	int timeout = atoi(argv[4]);

	printf("UDP Sender initializing...\n");
	printf("UDP Sender connecting to host %s on port %s.\nWindow size set to %d packets, and timeout set to %d seconds\n\n",
			hostname, port, window_size, timeout);

	int error = 0;						// error tracking
	int send_socket = 0;				// socket to transmit on
	struct addrinfo hints, *info, *p;	// obtaining socket address info
	int flags = 0;						// connection flagsc

	size_t max_buffer_size = MAX_BUFFER_LENGTH;	// max buffer size
	char *input_message = NULL;			// local input message (from stdin)
	int input_message_length = 0;		// length of input message

	packet_t packet = { 0 };
	int sequence_number = 0;

	int send_size = 0;
	int bytes_sent, bytes_sent_tmp = 0;	// bytes sent by sendto() call

	// prepare for a UDP socket using IPv4 on the local machine
   	memset(&hints, 0, sizeof(hints));
   	hints.ai_family = AF_INET;		// use IPv4
   	hints.ai_socktype = SOCK_DGRAM;	// UDP socket

	// obtain information about the location of available UDP sockets
	error = getaddrinfo(hostname, port, &hints, &info);
	if (error != 0) {
		fprintf(stderr, "Sender: getaddrinfo: %s\n", gai_strerror(error));
		return 2;
	}

	// loop through all the results and bind to the first we can
	for (p = info; p != NULL; p = p->ai_next) {
		send_socket = socket(p->ai_family, p->ai_socktype, p->ai_protocol);
		if (send_socket == -1) {
			perror("Sender: socket");
			continue;
		}

		// a socket was successfully bound! All done here
		break;
	}

	// ensure a socket was bound to
	if (p == NULL) {
		fprintf(stderr, "Sender: failed to bind\n");
		return 3;
	}

	// all done with this structure; free it
	freeaddrinfo(info);

	// begin talking
	debug(("Sender: Ready to transmit...\n"));

	while (1) {

		// reset buffers, pointers
		memset(&packet, 0, sizeof(packet));
		input_message = NULL;

		// reset counters
		bytes_sent = 0, bytes_sent_tmp = 0;
		input_message_length = 0;
		send_size = 0;

		// obtain user input message from stdin
		fputs("send>> ", stdout);
        getline(&input_message, &max_buffer_size, stdin);

		// Strip newline
        input_message_length = strnlen(input_message, MAX_BUFFER_LENGTH);
        input_message[input_message_length-1] = 0;

		// populate packet - add sequence number
		packet.sequence_number = sequence_number;
		strncpy(packet.message, input_message, input_message_length);
		send_size = input_message_length + sizeof(sequence_number);

		// TODO: add packet to FIFO queue

		// TODO: only send message if non-ACK'd messages are less than the window size

		// send the message
		do {
			// send over UDP
			debug(("About to send: %s -- Len: %d\n", packet.message, send_size));
			bytes_sent_tmp = sendto(send_socket,
									&((char*)&packet)[bytes_sent],
									send_size,
									flags,
									p->ai_addr,
									p->ai_addrlen);
			// check for errors
			if (bytes_sent_tmp <= 0) {
				printf("Sender: failed to send\n");
				exit(-1);
			}
			// increment counter
			bytes_sent += bytes_sent_tmp;
			send_size -= bytes_sent;
			debug(("Bytes sent: %d\n", bytes_sent));
		} while (send_size > 0);  // account for truncation during send

		// TODO: receive ACK

		// increment sequence number
		sequence_number++;

		// TODO: stop listening after timeout

	}

	return -1;
}
