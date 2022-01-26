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

#define TCP_SERVER_PORT		"30123"
#define TCP_SERVER_ADDRESS	"127.0.0.1"
#define SERVER_BACKLOG		15
#define MAX_STR_LENGTH		40
#define MAX_KEY_VAL_PAIRS	20

/** struct that contains a message to be transmitted and some metadata */
typedef struct _socket_info_t {
	int af;
	int socket_fd;
	struct sockaddr *socket_addr;
} socket_info_t;

typedef struct _key_value_pair_t {
	char key[MAX_STR_LENGTH];
	char value[MAX_STR_LENGTH];
} key_value_pair_t;

key_value_pair_t key_value_database[MAX_KEY_VAL_PAIRS] = { 0 };


int main (void)
{
	printf("Server initializing...\n");
	printf("Clients may connect to the TCP Server via Port %s on Address %s\n", TCP_SERVER_PORT, TCP_SERVER_ADDRESS);

	int listen_socket, new_socket;				// sockets for listening and connecting
	struct addrinfo hints, *addrinfo_list, *p;	// obtaining socket address info
	struct sockaddr_storage their_addr;			// the new connection's address
	socklen_t addr_size;						// size of incoming socket address
	int error;									// error checking
	socket_info_t *socket_addr;					// info and addr of new socket
	int connected;

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

			// allocate space for new socket_info_t struct
			socket_addr = (socket_info_t *)malloc(sizeof(socket_info_t));
			assert(socket_addr);
			memset(socket_addr, 0, sizeof(socket_info_t));

			// populate socket_addr struct for new thread to use
			socket_addr->af = their_addr.ss_family;
			socket_addr->socket_fd = new_socket;
			socket_addr->socket_addr = (struct sockaddr *)&their_addr;

			printf("New socket connection made! Socket: %03d\n", new_socket);

			connected = 1;

		// while connected, listen to what the client is saying
		} else {
			// TODO: implement listening to client over TCP
			// TODO: implement key-value lookup table
			// TODO: implement talking to client over TCP
		}

		// TODO: handle disconnections
	}
}
