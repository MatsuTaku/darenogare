#ifndef _COMMON_H_
#define _COMMON_H_

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <assert.h>
#include <math.h>

#define PORT			(u_short)51001

#define MAX_CLIENTS		4				
#define MAX_NAME_SIZE	10 				

#define MAX_DATA		200				

typedef enum {
		END_COMMAND
} COMMANDS;

typedef struct {
		int x;	// X座標
		int y;	// Y座標
} POSITION;

typedef struct {
		double vx;	// 速度成分X
		double vy;	// 速度成分Y
} VEROCITY;

typedef struct  {
		int type;		// オブジェクト型（キャラ、アイテム、障害物）
		int id;			// オブジェクト番号
		POSITION pos;	// 座標
} OBJECT;

typedef struct {
		int item;			// 所有アイテム
		double dir;			// 進行方向
		VEROCITY ver;		// 速度ベクトル
		int alive;			// 生存フラグ
} PLAYER;

// 通信データ
typedef struct {
		int id;				// ユーザーID
		PLAYER player;		// プレイヤーのデータ
} entityState;

#endif
