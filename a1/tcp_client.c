/**
 * CMPT 434 Assignment 1
 * Monday January 31 2022
 * Tyrel Kostyk - tck290 - 11216033
 */

 #include <netdb.h>
 #include <stdio.h>
 #include <stdlib.h>
 #include <string.h>
 #include <unistd.h>

 #include <sys/socket.h>
 #include <sys/types.h>

 #define MAX_BUFFER_SIZE		1024

 /**
  * Outputs help text for the sender program.
  */
 void printSenderHelpText (void) {
     const char *help =
     "Usage: sender <remote-name> <port>\n"
     "\n"
     "    remote-name: The hostname or IP address of the server\n"
     "    port: The port number to use for the server connection\n";

     puts(help);
 }


 /**
  * Performs connection boilerplate for a client program.
  *
  * :param hostname: The name or IP address of the server
  * :param port: The port or service name to connect with
  * :param socket_fd: A pointer which will contain the socket file descriptor upon a
  *   successful connection
  * :return: 0 on success, 2 on address info failure, 3 on failure to connect
  */
 int clientConnect (char* hostname, char* port, int* socket_fd) {

     int status, sock_fd;
     struct addrinfo hints;
     struct addrinfo *info, *p;

     memset(&hints, 0, sizeof hints);
     hints.ai_family = AF_UNSPEC;
     hints.ai_socktype = SOCK_STREAM;

     if ((status = getaddrinfo(hostname, port, &hints, &info)) != 0) {
         fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(status));
         return 2;
     }

     for (p = info; p != NULL; p = p->ai_next) {
         if ((sock_fd = socket(p->ai_family, p->ai_socktype, p->ai_protocol)) == -1) {
             perror("client: socket");
             continue;
         }

         if (connect(sock_fd, p->ai_addr, p->ai_addrlen) == -1) {
             close(sock_fd);
             perror("client: connect");
             continue;
         }

         break;
     }

     if (p == NULL) {
         fprintf(stderr, "client: Connection failed\n");
         return 3;
     }

     freeaddrinfo(info);

     *socket_fd = sock_fd;

     return 0;
 }


 /**
  * Performs disconnection boilerplate for a client program.
  *
  * :param socket_fd: A pointer which contains the socket file descriptor to
  *   close/disconnect
  * :return: 0 on success, -1 on failure
  */
 int clientDisconnect (int* socket_fd) {
     if (close(*socket_fd) == -1) {
         perror("client: close");
         return -1;
     }
     return 0;
 }


 /**
  * Connects to a server with hostname specified in argv[1] and port argv[2]. Sends
  * lines of text from stdin to the server.
  */
 int main (int argc, char* argv[]) {

     if (argc != 3) {
         printSenderHelpText();
         return 1;
     }

     int conn_status;
     int *socket = (int*)malloc(sizeof(int));

     // Try to connect and return error status if a problem occurs
     if ((conn_status = clientConnect(argv[1], argv[2], socket)) != 0)
         return conn_status;

     int num_sent = 0;
     size_t str_length = 0;
	 char str[MAX_BUFFER_SIZE] = { 0 };
	 size_t sz = MAX_BUFFER_SIZE;
     int num_rcvd = 0;

     // Spin until user types quit or exit
     while (1) {

		 // SEND COMMAND TO SERVER

		 // reset string buffer
		 memset(str, 0, sizeof(str));

         puts("send>> ");
         getline((char **)(&str), &sz, stdin);

         // if (strcmp(str, "quit\n") == 0 || strcmp(str, "exit\n") == 0)
         //     break;

         // strip newline
         str_length = strlen(str);
         str[str_length-1] = 0;

         // Send the string
         num_sent = 0;
		 int num_sent_tmp = 0;
         do {
             printf("Str: %s -- Len: %lu\n", &str[num_sent], str_length);
             num_sent_tmp = send(*socket, &str[num_sent], str_length, 0);
			 num_sent += num_sent_tmp;
             str_length -= num_sent_tmp;
             printf("Num sent: %d\n", num_sent_tmp);
         } while (str_length > 0);  // Account for truncation during send

		 // reset string buffer
		 memset(str, 0, sizeof(str));

		 // RECEIVE RESPONSE

		 num_rcvd = recv(*socket, str, MAX_BUFFER_SIZE, 0);
		 if (num_rcvd > 0)
			 printf("%s\n", str);
		 else if (num_rcvd <= -1)
			 perror("receiver: recv");
		 else {
			 puts("receiver: Connection lost");
			 break;
		 }
     }

     if ((conn_status = clientDisconnect(socket)) == -1)
         return conn_status;

     free(socket);

     return 0;
 }
