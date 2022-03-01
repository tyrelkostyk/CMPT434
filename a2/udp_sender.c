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
                             PRIVATE FUNCTION STUBS
*******************************************************************************/

static int windowSize(void);

static void packetPrepare(void);
static void packetSend(struct addrinfo *p);
static void packetReceive(struct addrinfo *p);

static void fifoPush(const packet_t* packet);
static void fifoPop(packet_t* packet);
static void fifoPeekNewest(packet_t* packet);
static int fifoEmpty(void);
static int fifoFull(void);


/*******************************************************************************
                         PRIVATE DEFINITIONS AND GLOBALS
*******************************************************************************/

#define SEQUENCE_NUMBER	(packet_fifo_write_cursor)	// index of FIFO used as sequence number

packet_t packet_fifo[FIFO_SIZE] = { 0 };	// sender FIFO queue
int packet_fifo_write_cursor = 0;			// write cursor for FIFO
int packet_fifo_read_cursor = 0;			// read cursor for FIFO

int sender_socket = 0;	// socket to transmit on
int flags = 0;			// connection flags

int upper_sequence_number = 0;	// top of window; latest non-ACK'd sent packet
int lower_sequence_number = 0;	// bottom of window; oldest non-ACK'd sent packet


/*******************************************************************************
                                      MAIN
*******************************************************************************/

int main(int argc, char* argv[])
{
	// confirm number of input arguments
	if (argc != 5) {
		printf("Error: invalid input received\n");
		exit(-1);
	}

	// obtain settings from input arguments
	char* hostname = argv[1];
	char *port = argv[2];
	int window_size_limit = atoi(argv[3]);
	int timeout = atoi(argv[4]);

	// confirm port number is within valid bounds
	if (atoi(port) < PORT_MIN || atoi(port) > PORT_MAX) {
		printf("Error: port number not valid\n");
		exit(-1);
	}

	// confirm window size limit does not exceed size of FIFO Queue
	if (window_size_limit > FIFO_SIZE) {
		printf("Error: window size limit is too large\n");
		exit(-1);
	}

	printf("UDP Sender initializing...\n");
	printf("UDP Sender connecting to host %s on port %s.\nWindow size set to %d packets, and timeout set to %d seconds\n\n",
			hostname, port, window_size_limit, timeout);

	int error = 0;						// error tracking
	struct addrinfo hints, *info, *p;	// obtaining socket address info

	// SET UP A UDP SOCKET

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
		sender_socket = socket(p->ai_family, p->ai_socktype, p->ai_protocol);
		if (sender_socket == -1) {
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

	// BEGIN SENDING

	debug(("Sender: Ready to transmit...\n"));

	while (1) {

		// obtain new packet to send
		packetPrepare();

		// send available packet from FIFO (if window size not exceeded)
		if (windowSize() <= window_size_limit)
			packetSend(p);

		// receive ACK, remove ACK'd packets from FIFO
		packetReceive(p);

		// TODO: begin re-transmissions after timeout

	}
}


/*******************************************************************************
                               PRIVATE FUNCTIONS
*******************************************************************************/

/**
 * Provides the current size of the sender's window. Accounts for wrap-around.
 */
static int windowSize(void)
{
	if (upper_sequence_number >= lower_sequence_number)
		return upper_sequence_number - lower_sequence_number;

	return lower_sequence_number - upper_sequence_number;
}


/**
 * Receive a new message from stdin, store as packet in FIFO Queue.
 */
static void packetPrepare(void)
{
	// prompt user for input from stdin
	fputs("send>> ", stdout);

	// obtain user input message from stdin
	size_t max_message_size = MAX_MESSAGE_LENGTH;	// max buffer size
	char *input_message = NULL;						// pointer to input message
	getline(&input_message, &max_message_size, stdin);

	// Strip newline
	int input_message_length = strnlen(input_message, MAX_MESSAGE_LENGTH);
	input_message[input_message_length-1] = 0;

	// populate packet - add sequence number
	packet_t packet = { 0 };
	packet.sequence_number = SEQUENCE_NUMBER;
	strncpy(packet.message, input_message, input_message_length);

	// push packet onto FIFO queue
	debug(("packetPrepare: about to push <%d: %s> onto FIFO\n", packet.sequence_number, packet.message));
	fifoPush(&packet);
}


/**
 * Send a packet (from FIFO Queue) over UDP.
 * @param p Socket addr info.
 */
static void packetSend(struct addrinfo *p)
{
	// confirm that input pointer is valid
	if (p == 0) {
		debug(("packetSend: input pointer is NULL\n"));
		exit(-1);
	}

	// obtain oldest packet from FIFO
	packet_t packet = { 0 };
	fifoPeekNewest(&packet);

	// determine size of message
	int send_size = strnlen(packet.message, MAX_MESSAGE_LENGTH) + HEADER_SIZE;

	// send the message
	do {
		debug(("packetSend: about to send: %s -- Len: %d\n", packet.message, send_size));

		// send over UDP
		int bytes_sent = 0;
		int bytes_sent_tmp = 0;
		bytes_sent_tmp = sendto(sender_socket,
								&((char*)&packet)[bytes_sent],
								send_size,
								flags,
								p->ai_addr,
								p->ai_addrlen);

		// check for errors
		if (bytes_sent_tmp <= 0) {
			printf("packetSend: failed to send (%d)\n", bytes_sent_tmp);
			exit(-1);
		}

		// increment counter
		bytes_sent += bytes_sent_tmp;
		send_size -= bytes_sent;
	} while (send_size > 0);  // account for truncation during send

	// increment upper bound of window
	upper_sequence_number++;
	if (upper_sequence_number == SEQUENCE_NUMBER_MAX)
		upper_sequence_number = 0;
}


/**
 * Receive a new ACK packet over UDP, popping ACK'd packets from the FIFO Queue.
 */
static void packetReceive(struct addrinfo *p)
{
	// receive ACK
	packet_t packet = { 0 };
	int bytes_received = recvfrom(sender_socket,
							  	  (char*)&packet,
							  	  MAX_BUFFER_LENGTH-1,
							  	  flags,
							  	  NULL,
							  	  NULL);

	if (bytes_received <= 0) {
		printf("packetReceive: failed to receive ACK\n");
		exit(-1);
	}

	// retrieve sequence number from packet
	int next_expected_sequence_number = packet.sequence_number;

	debug(("packetReceive: received ACK; Up to %d are now ACK'd\n", next_expected_sequence_number));

	// pop all ACK'd packets off of FIFO
	while (lower_sequence_number != next_expected_sequence_number) {
		// pop oldest packet off of FIFO
		fifoPop(NULL);

		// increment lower bound of window
		lower_sequence_number++;
		if (lower_sequence_number == SEQUENCE_NUMBER_MAX)
			lower_sequence_number = 0;
	}

	debug(("packetReceive: window size is now %d\n", windowSize()));
}


/**
 * Place a new packet into the FIFO Queue.
 * @param packet Pointer to a valid packet.
 */
static void fifoPush(const packet_t* packet)
{
	// confirm that packet pointer is valid
	if (packet == 0) {
		debug(("fifoPush: input pointer is NULL\n"));
		exit(-1);
	}

	// confirm that there is room within FIFO
	if (fifoFull()) {
		debug(("fifoPush: FIFO is full\n"));
		exit(-1);
	}

	// erase previous contents of FIFO entry
	memset(&packet_fifo[packet_fifo_write_cursor],
		   0,
		   sizeof(packet_fifo[packet_fifo_write_cursor]));

	// determine size of packet
	int packet_size = strnlen(packet->message, MAX_MESSAGE_LENGTH) + HEADER_SIZE;

	// place new packet into FIFO
	memcpy(&packet_fifo[packet_fifo_write_cursor],
		   packet,
		   packet_size);

	// increment FIFO write cursor
	packet_fifo_write_cursor++;
	if (packet_fifo_write_cursor == FIFO_SIZE)
		packet_fifo_write_cursor = 0;

	debug(("fifoPush: pushed packet; Write Cursor is now %d\n", packet_fifo_write_cursor));
}


/**
 * Extract (and remove) a packet from the FIFO Queue.
 * @param packet Pointer that packet will be extracted into. Set to NULL to ignore.
 */
static void fifoPop(packet_t* packet)
{
	// confirm that there is a valid packet within FIFO
	if (fifoEmpty()) {
		debug(("fifoPop: FIFO is empty\n"));
		exit(-1);
	}

	// determine size of packet
	int packet_size = strnlen(packet_fifo[packet_fifo_read_cursor].message, MAX_MESSAGE_LENGTH) + HEADER_SIZE;

	// extract packet from FIFO
	if (packet != NULL) {
		memcpy(packet,
			   &packet_fifo[packet_fifo_read_cursor],
			   packet_size);
	}

	// erase packet contents from FIFO
	memset(&packet_fifo[packet_fifo_read_cursor],
		   0,
		   sizeof(packet_fifo[packet_fifo_write_cursor]));

	// increment FIFO read cursor
	packet_fifo_read_cursor++;
	if (packet_fifo_read_cursor == FIFO_SIZE)
		packet_fifo_read_cursor = 0;

	debug(("fifoPop: popped packet; Read Cursor is now %d\n", packet_fifo_read_cursor));
}


/**
 * Extract (but don't remove) the newest packet from the FIFO Queue.
 * @param packet Pointer that packet will be extracted into.
 */
static void fifoPeekNewest(packet_t* packet)
{
	// confirm that packet pointer is valid
	if (packet == 0) {
		debug(("fifoPeekNewest: input pointer is NULL\n"));
		exit(-1);
	}

	// confirm that there is a valid packet within FIFO
	if (fifoEmpty()) {
		debug(("fifoPeekNewest: FIFO is empty\n"));
		exit(-1);
	}

	// determine size of packet
	int packet_size = strnlen(packet_fifo[packet_fifo_write_cursor-1].message, MAX_MESSAGE_LENGTH) + HEADER_SIZE;

	// extract packet from FIFO
	memcpy(packet,
		   &packet_fifo[packet_fifo_write_cursor-1],
		   packet_size);
}


/**
 * Indicates if the FIFO is empty.
 *
 * FIFO is said to be empty if there is no message data available in the packet
 * being pointed to by the read cursor.
 */
static int fifoEmpty(void) {
	return ((strnlen(packet_fifo[packet_fifo_read_cursor].message, MAX_MESSAGE_LENGTH) == 0));
}


/**
 * Indicates if the FIFO is full.
 *
 * FIFO is said to be full if the write cursor has wrapped around back to the
 * read cursor, AND there is still data being pointed to by the read cursor.
 */
 static int fifoFull(void) {
	return ((packet_fifo_write_cursor == packet_fifo_read_cursor)
		&&  (strnlen(packet_fifo[packet_fifo_read_cursor].message, MAX_MESSAGE_LENGTH) > 0));
}
