#ifndef _CLIENT_COMMON_H_
#define _CLIENT_COMMON_H_

#include"../common.h"

#define BOOST_ACCELE	1	//	one frame

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
/* client_win.c */
extern PLAYER player; //プレイヤー(自分)のステータス

#define ALL_CLIENTS	-1

#endif
