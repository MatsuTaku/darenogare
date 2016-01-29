#ifndef _SERVER_COMMON_H_
#define _SERVER_COMMON_H_

#include"../common.h"

enum generateFrequency {	// 毎秒の頻度
		FREQ_OBSTACLE = 4,
		FREQ_ITEM = 1,
};

enum itemWeight {
		WEIGHT_NOIZE = 12,
		WEIGHT_LASER = 8,
		WEIGHT_MISSILE = 17,
		WEIGHT_MINIMUM = 15,
		WEIGHT_BARRIER = 14,
		WEIGHT_NUM,
};

typedef struct {
		eventNotification eventStack[MAX_CLIENTS][MAX_EVENT];
} eventAssembly;

/* server_net.c */
extern ITEM_NUMBER Item[ITEM_NUM];
extern PLAYER player;
extern POSITION positon;
#define ALL_CLIENTS	-1   
/* server_command.c */
/* serer_system.c */
extern OBJECT allOBject[MAX_OBJECT];
extern PLAYER allPlayer[MAX_CLIENTS];
extern OBSTACLE allObstacle[MAX_OBSTACLE];
extern ITEM allItem[MAX_ITEM];

#endif
