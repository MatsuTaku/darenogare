#ifndef _CLIENT_COMMON_H_
#define _CLIENT_COMMON_H_

#include "../common.h"

#define ALL_CLIENTS		-1
#define BOOST_ACCELE	1	// one frame
extern entityState player;
extern mapData MAP;

enum axisDirection {
		LEFT,
		RIGHT,
		UP,
		DOWN
};

enum boostDirection {
		GO,
		BACK,
		NEUTRAL
};

/* client_net.c */
extern int clientID;

#endif
