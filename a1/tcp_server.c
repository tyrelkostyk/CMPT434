/**
 * CMPT 434 Assignment 1
 * Monday January 31 2022
 * Tyrel Kostyk - tck290 - 11216033
**/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>

#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>

#include "defs.h"


/** struct that contains socket info (for easier passing of socket info) */
typedef struct _socket_info_t {
	int af;
	int socket_fd;
	struct sockaddr *socket_addr;
} socket_info_t;

/** struct that contains a string key value pair */
typedef struct _key_value_pair_t {
	char key[MAX_STR_LENGTH];
	char value[MAX_STR_LENGTH];
} key_value_pair_t;

/** struct that contains a string command and arguments */
typedef struct _command_t {
	char cmd[MAX_STR_LENGTH];
	char arg1[MAX_STR_LENGTH];
	char arg2[MAX_STR_LENGTH];
} command_t;

/** local database that contains all of the key-value pairs */
key_value_pair_t key_value_database[MAX_KEY_VAL_PAIRS] = { 0 };


static const char *getValue(const char *key)
{
	// loop through entire database until a match is found
	for (int i = 0; i < sizeof(key_value_database); i++) {
		// match found -> return value!
		if (strncmp(key_value_database[i].key, key, MAX_STR_LENGTH) == 0)
			return key_value_database[i].value;
	}

	// no match found -> return NULL
	return NULL;
}


static void add(const char *key, const char *value)
{
	// loop through entire database until an open spot is found
	for (int i = 0; i < sizeof(key_value_database); i++) {
		// open spot found -> place value!
		if (key_value_database[i].key == NULL) {
			strncpy(key_value_database[i].key, key, MAX_STR_LENGTH);
			strncpy(key_value_database[i].value, value, MAX_STR_LENGTH);
			return;
		}
	}

	// no match found
	return;
}


static void removeKey(const char *key)
{
	// loop through entire database until a match is found
	for (int i = 0; i < sizeof(key_value_database); i++) {
		// match found -> delete key-value pair
		if (strncmp(key_value_database[i].key, key, MAX_STR_LENGTH) == 0) {
			memset(key_value_database[i].key, 0, MAX_STR_LENGTH);
			memset(key_value_database[i].value, 0, MAX_STR_LENGTH);
			return;
		}
	}

	// no match found
	return;
}


static void extractArguments(const char *input, command_t *command, int size)
{
	// input pointer must be valid; size must be positive
	if (input == NULL || command == NULL || size <= 0)
		return;

	// reset previously extracted command arguments
	memset(command, 0, sizeof(command_t));

	int index = 0;
	const char *input_copy = input;
	int size_remaining = size;

	// move cursor to end of command name (i.e. arg0)
	while ((index < size_remaining)
		&& (memcmp(&input_copy[index], " ", 1) != 0)
		&& (memcmp(&input_copy[index], "\0", 1) != 0))
	{
		index++;
	}

	// if we're out of room -> exit
	if (index >= size_remaining)
		return;

	// copy command name into command struct (if ended on space or term byte)
	strncpy(command->cmd, input_copy, index);

	// update local cursor variables to point to start of next argument
	size_remaining -= index + 1;
	input_copy = &input[index + 1];
	index = 0;

	// move cursor to end of first argument (i.e. arg1)
	while ((index < size_remaining)
		&& (memcmp(&input_copy[index], " ", 1) != 0)
		&& (memcmp(&input_copy[index], "\0", 1) != 0))
	{
		index++;
	}

	// if we're out of room -> exit
	if (index >= size_remaining)
		return;

	// copy first argument into command struct
	strncpy(command->arg1, input_copy, index);
	// update local cursor variables to point to start of next argument
	size_remaining -= index + 1;
	input_copy = &input[index + 1];
	index = 0;

	// move cursor to end of second argument (i.e. arg2)
	while ((index < size_remaining)
		&& (memcmp(&input_copy[index], " ", 1) != 0)
		&& (memcmp(&input_copy[index], "\0", 1) != 0))
	{
		index++;
	}

	// if we're out of room -> exit
	if (index >= size_remaining)
		return;

	// copy second argument into command struct
	strncpy(command->arg2, input_copy, index);

	return;
}


int main (void)
{
	printf("Server initializing...\n");
	printf("Clients may connect to the TCP Server via Port %s on Address %s\n", TCP_SERVER_PORT, TCP_SERVER_ADDRESS);

	int listen_socket, new_socket;				// sockets for listening and connecting
	struct addrinfo hints, *addrinfo_list, *p;	// obtaining socket address info
	struct sockaddr_storage their_addr;			// the new connection's address
	socklen_t addr_size;						// size of incoming socket address
	int error;									// error checking
	socket_info_t socket_addr = { 0 };			// info and addr of new socket
	int connected = 0;							// flag representing if a connection exists
	int bytes_received = 0;						// bytes read by recv() call
	int bytes_remaining = 0;					// bytes to be sent by send() call
	int bytes_sent = 0;							// bytes sent by send() call
	char recv_buffer[MAX_BUFFER_LENGTH];		// local receive buffer
	char send_buffer[MAX_BUFFER_LENGTH];		// local transmit buffer
	command_t recv_commands = { 0 };			// extracted command arguments

	memset(&hints, 0, sizeof(hints));
	hints.ai_family = AF_UNSPEC;		// don't care if IPv4 or IPv6
	hints.ai_socktype = SOCK_STREAM;	// TCP socket
	hints.ai_flags = AI_PASSIVE;		// use host machine IP

	// obtain information about the location of available TCP sockets
	error = getaddrinfo(NULL, TCP_SERVER_PORT, &hints, &addrinfo_list);
	if (error) {
		fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(error));
		exit(-1);
	}

	// loop through all the results and bind to the first we can
	for (p = addrinfo_list; p != NULL; p = p->ai_next) {
		listen_socket = socket(p->ai_family, p->ai_socktype, p->ai_protocol);
		if (listen_socket == -1) {
			perror("server: socket");
			continue;
		}

		// prevent annoying "Addr in Use" error
		int yes=1;
		if (setsockopt(listen_socket, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(int)) == -1) {
			perror("setsockopt");
			exit(1);
		}

		if (bind(listen_socket, p->ai_addr, p->ai_addrlen) == -1) {
			close(listen_socket);
			perror("server: bind");
			continue;
		}

		// a socket was successfully bound! All done here
		break;
	}

	// all done with this structure; free it
	freeaddrinfo(addrinfo_list);

	// ensure a socket was bound to
	if (p == NULL) {
		fprintf(stderr, "server: failed to bind\n");
		exit(-1);
	}

	// begin listening on this socket for new connections
	if (listen(listen_socket, SERVER_BACKLOG) == -1) {
		perror("listen");
		exit(-1);
	}

	while (1) {

		// make a new connection if one doesn't exist
		if (!connected) {

			// attempt to connect
			addr_size = sizeof(their_addr);
			new_socket = accept(listen_socket, (struct sockaddr *)&their_addr, &addr_size);

			// reset socket_info_t struct
			memset(&socket_addr, 0, sizeof(socket_info_t));

			// populate socket_addr struct for new thread to use
			socket_addr.af = their_addr.ss_family;
			socket_addr.socket_fd = new_socket;
			socket_addr.socket_addr = (struct sockaddr *)&their_addr;

			connected = 1;

			debug(("New socket connection made! Socket: %03d\n", new_socket));

		// while connected, listen to what the client is saying
		} else {

			// reset receive and transmit buffers
			memset(&recv_buffer, 0, sizeof(recv_buffer));
			memset(&send_buffer, 0, sizeof(send_buffer));

			// receive incoming string message
			bytes_received = recv(socket_addr.socket_fd, recv_buffer, MAX_BUFFER_LENGTH-1, 0);

			// if connection closed; close connection
			if (bytes_received <= 0) {
				debug(("Failed to communicate with sender client; closing connection...\n"));
				close(socket_addr.socket_fd);
				connected = 0;

			} else {
				debug(("Message received from sender client! Socket %d, Length %d\n", socket_addr.socket_fd, bytes_received));
			}

			// extract arguments from received message
			extractArguments(recv_buffer, &recv_commands, bytes_received);

			bytes_remaining = 0;
			if (memcmp(recv_commands.cmd, "add", sizeof("add")) == 0) {
				add(recv_commands.arg1, recv_commands.arg2);

			} else if (memcmp(recv_commands.cmd, "getvalue", sizeof("getvalue")) == 0) {
				const char *value = NULL;
				value = getValue(recv_commands.arg1);

				if (value != NULL) {
					strncpy(send_buffer, value, MAX_STR_LENGTH);
					bytes_remaining = strnlen(send_buffer, MAX_BUFFER_LENGTH);
				} else {
					debug(("Error: Key %s does not exist in database\n", recv_commands.arg1));
					strncpy(send_buffer, "Error: Key does not exist in database", MAX_STR_LENGTH);
					bytes_remaining = strnlen(send_buffer, MAX_BUFFER_LENGTH);
				}

			} else if (memcmp(recv_commands.cmd, "getall", sizeof("getvalue")) == 0) {
				// TODO
				// bytes_remaining = strnlen(send_buffer, MAX_BUFFER_LENGTH);

			} else if (memcmp(recv_commands.cmd, "remove", sizeof("remove")) == 0) {
				removeKey(recv_commands.arg1);

			} else if (memcmp(recv_commands.cmd, "quit", sizeof("quit")) == 0) {
				printf("Client requested QUIT - Goodbye\n");
				close(socket_addr.socket_fd);
				exit(0);

			} else {
				debug(("Error: Input %s was not recognized as a valid command\n", recv_buffer));
			}

			if (bytes_remaining > 0) {
				bytes_sent = send(socket_addr.socket_fd,
					send_buffer,
					bytes_remaining,
					MSG_NOSIGNAL);

				// if error or connection closed; kill this thread
				if (bytes_sent <= 0) {
					debug(("Failed to communicate with client; disconnecting...\n"));
					close(socket_addr.socket_fd);
					connected = 0;

				} else {
					debug(("Message sent to client! Socket %d\n", socket_addr.socket_fd));
				}
			}
		}
	}
}
