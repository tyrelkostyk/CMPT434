/**
 * CMPT 434 Assignment 2
 * Monday February 28 2022
 * Tyrel Kostyk - tck290 - 11216033
 */

#ifndef _DEFS_H_
#define _DEFS_H_


#ifdef DEBUG
#define debug(x) printf x
#else
#define debug(x) do {} while (0)
#endif

/** SHARED DEFINITIONS **/
#define LOCAL_ADDRESS		"127.0.0.1"
#define PORT_MIN			30000
#define PORT_MAX			40000
#define MAX_BUFFER_LENGTH	1024
#define HEADER_SIZE			(sizeof(int))
#define MAX_MESSAGE_LENGTH	(MAX_BUFFER_LENGTH-HEADER_SIZE)
#define FIFO_SIZE			(8 << 1)
#define SEQUENCE_NUMBER_MAX	(FIFO_SIZE)

typedef struct _packet_t {
	int sequence_number;
	char message[MAX_MESSAGE_LENGTH];
} packet_t;


#endif  // _DEFS_H_
