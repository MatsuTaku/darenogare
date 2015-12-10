#ifndef _CLIENT_COMMON_H_
#define _CLIENT_COMMON_H_

#include "../common.h"

#define ALL_CLIENTS		-1
#define BOOST_ACCELE	1	// one frame
#define PI				3.14159265
#define HALF_DEGRESS	180	
#define ANGULAR_VEROCITY	360	// 角速度
#define ACCELE_GO	140		// 加速度
#define ACCELE_BRAKE	-100	// 減速度
#define MAXIMUM_SPEED	40
#define MAXIMUM_SPEED_OBSTACLE	60
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

extern entityStateSet rocket;
extern mapData MAP;

/* client_system.c */
extern ASSEMBLY allAssembly;
extern OBJECT* object;
extern PLAYER* player;
extern PLAYER* myPlayer;
/* client_net.c */
extern int clientID;


#endif
