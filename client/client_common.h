#ifndef _CLIENT_COMMON_H_
#define _CLIENT_COMMON_H_

#include "../common.h"

#define ALL_CLIENTS		-1
#define BOOST_ACCELE	1	// one frame
#define PI				3.141592
#define HALF_DEGRESS	180	
#define ANGULAR_VEROCITY	100	// 秒間100度
#define ACCELE_GO	10		// 加速度
#define ACCELE_BRAKE	-15	// 減速度
#define RESISTANCE		-0	// 抵抗

extern entityState rocket;
extern mapData MAP;

/* client_net.c */
extern int clientID;
extern Charatype gChara[CT_NUM];
extern ITEM_NUMBER Item[ITEM_NUM];
extern PLAYER player;
extern POSITION positon;
#endif
