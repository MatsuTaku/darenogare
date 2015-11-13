#ifndef _COMMON_H_
#define _COMMON_H_

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <sys/types.h>
#include <assert.h>
#include <math.h>

#define PORT			(u_short)51001

#define MAX_CLIENTS		4				
#define MAX_NAME_SIZE	10 				

#define MAX_DATA		200				
#define MAX_OBJECT		0xff

typedef enum {
		END_COMMAND
} COMMANDS;

// オブジェクトタイプ列挙体
typedef enum {
		EMPTY,
		CHARACTER,
		ITEM,
		OBSTACLE
} OBJECT_TYPE;

typedef struct {
		int x;	// X座標
		int y;	// Y座標
} POSITION;

typedef struct  {
		OBJECT_TYPE type;		// オブジェクト型（キャラ、アイテム、障害物）
		int id;			// オブジェクト番号
		POSITION pos;	// 座標
		void* typeBuffer;	// オブジェクト固有の構造体 *キャスト必須
} OBJECT;

typedef struct {
		double vx;	// 速度成分X
		double vy;	// 速度成分Y
} VEROCITY;

typedef struct {
		int item;			// 所有アイテム
		double dir;			// 進行方向
		VEROCITY ver;		// 速度ベクトル
		int alive;			// 生存フラグ
} PLAYER;

typedef enum {
		RANGE_CHARACTER = 20,
		RANGE_ITEM = 15,
		RANGE_ROCK = 10,
}	OBJECT_RANGE;

typedef enum {
		ITEM_EMPTY		= 0,
		ITEM_THUNDER	= 1,
		ITEM_LASER		= 2,
		ITEM_TRAP		= 3,
		ITEM_MINIMUM	= 4, 
}ITEM_NUM;

// 通信データ(client > server)
typedef struct {
		int id;				// ユーザーID
		PLAYER player;		// プレイヤーのデータ
} entityState;

#endif
