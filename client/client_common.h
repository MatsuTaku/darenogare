#ifndef _CLIENT_COMMON_H_
#define _CLIENT_COMMON_H_

#include "../common.h"

#define ALL_CLIENTS		-1
#define BOOST_ACCELE	1	// one frame
#define PI				3.141592
#define HALF_DEGRESS	180	
#define ANGULAR_VEROCITY	180	// 秒間100度
#define ACCELE_GO	10		// 加速度
#define ACCELE_BRAKE	-5	// 減速度
// #define RESISTANCE		-0	// 抵抗


// ジョイパッドのボタン一覧
enum joypadButton {
		BUTTON_SQUARE	= 0,
		BUTTON_TRIANGLE	= 1,
		BUTTON_CROSS	= 2,
		BUTTON_CIRCLE	= 3,
		BUTTON_L1		= 4,
		BUTTON_R1		= 5,
		BUTTON_L2		= 6,
		BUTTON_R2		= 7,
		BUTTON_L3		= 8,
		BUTTON_R3		= 9,
		BUTTON_SELECT	= 10,
		BUTTON_START	= 11
};

extern entityState rocket;
extern mapData MAP;

/* client_net.c */
extern int clientID;
extern ITEM_NUMBER Item[ITEM_NUM];
extern PLAYER player;
extern POSITION positon;


#endif
