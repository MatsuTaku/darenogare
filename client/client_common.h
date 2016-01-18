#ifndef _CLIENT_COMMON_H_
#define _CLIENT_COMMON_H_

#include "../common.h"

#define ALL_CLIENTS		-1
#define BOOST_ACCELE	1	// one frame
#define ANGULAR_VEROCITY	360	// 角速度
#define ACCELE_GO	400		// 加速度
#define ACCELE_BRAKE	-300	// 減速度
#define MAXIMUM_SPEED	800
// #define RESISTANCE		-0	// 抵抗
#define WARN_TIME_RIMIT	10	// 帰還制限時間（秒）
#define FIRE_TIME_LASER 3	// レーザー照射時間
#define MODE_TIME_MINIMUM 12
#define MODE_TIME_BARRIER 6
#define MAX_MISSILE  5	// ミサイル最大数


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

enum deltaFrame {
		FRAME_LAST,
		FRAME_LATEST,
		FRAME_NUM
};


/* client_system.c */
extern ASSEMBLY allAssembly;
extern OBJECT* object;
extern PLAYER* player;
extern PLAYER* myPlayer;
/* client_net.c */

#endif
