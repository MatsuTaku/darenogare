#ifndef _COMMON_H_
#define _COMMON_H_

#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include<sys/types.h>
#include<assert.h>
#include<math.h>

#define PORT			(u_short)51771	

#define MAX_CLIENTS		2				
#define MAX_NAME_SIZE	10 				

#define MAX_DATA		200				

#define MAX_RESULT		64

#define END_COMMAND		'E'		  		
#define RESULT_COMMAND	'L'
#define ROCK_COMMAND	'R'
#define SCISSORS_COMMAND	'S'
#define PAPER_COMMAND	'P'
#define DEFAULT_COMMAND	'D'

#define EVEN	-1

typedef struct {
		char command;
		char commands[MAX_CLIENTS];
		int winner;
}	RESULT;


#endif
