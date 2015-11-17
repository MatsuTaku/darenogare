
#ifndef _SERVER_COMMON_H_
#define _SERVER_COMMON_H_

#include"../common.h"

entityState plane[4];
mapData MAP;

/* server_net.c */
extern int gClientNum;
extern Charatype gChara[CT_NUM];
extern ITEM_NUMBER Item[ITEM_NUM];
extern PLAYER player;
extern POSITION positon;
#define ALL_CLIENTS	-1   

#endif
