#ifndef _CLIENT_COMMON_H_
#define _CLIENT_COMMON_H_

#include "../common.h"

#define ALL_CLIENTS		-1
#define BOOST_ACCELE	1	// one frame
#define PI				3.141592
#define HALF_DEGRESS	180	
#define ANGULAR_VEROCITY	100	// 秒間100度

extern entityState rocket;
extern mapData MAP;

/* client_net.c */
extern int clientID;

#endif
