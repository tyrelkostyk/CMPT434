/**
 * CMPT 434 Assignment 1
 * Monday January 31 2022
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
#define SERVER_BACKLOG		15
#define MAX_STR_LENGTH		40
#define MAX_KEY_VAL_PAIRS	20
#define MAX_BUFFER_LENGTH	1024
#define TCP_SERVER_PORT		"30123"
#define TCP_SERVER_ADDRESS	"127.0.0.1"


#endif  // _DEFS_H_
