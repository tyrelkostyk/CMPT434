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
#define MAX_BUFFER_LENGTH	1024
#define LOCAL_ADDRESS		"127.0.0.1"

// typedef struct _packet_t {
// 	int sequence_number;
// 	char* message;
// 	// char message[MAX_BUFFER_LENGTH];
// } packet_t;

#endif  // _DEFS_H_
