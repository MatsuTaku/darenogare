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

#define FPS	60

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

/* プレイヤー */
typedef enum {
		CT_PLAYER1	= 0,
		CT_PLAYER2	= 1,
		CT_PLAYER3	= 2,
		CT_PLAYER4	= 3,      
		CT_NUM		= 4,	/* タイプの数 */
} Charatype;


/* OBJECT Values */
typedef struct {//マップの座標
		int x;	// X座標
		int y;	// Y座標
} POSITION;

typedef struct  {
		OBJECT_TYPE type;	// オブジェクト型（キャラ、アイテム、障害物）
		int id;				// オブジェクト番号
		POSITION pos;		// 座標
		void* typeBuffer;	// オブジェクト固有の構造体 *キャスト必須
} OBJECT;


/* PLAYER Values */
typedef enum {
		ITEM_THUNDER	= 0,
		ITEM_LASER  	= 1,
		ITEM_TRAP		= 2,
		ITEM_MINIMUM	= 3,
		ITEM_NUM		= 4
} ITEM_NUMBER;

typedef struct {
		double vx;	// 速度成分X
		double vy;	// 速度成分Y
} VEROCITY;

typedef enum {
		BOOST_NEUTRAL,
		BOOST_GO,
		BOOST_BACK
} BOOST_FLAG;

typedef enum {
		ROTATE_NEUTRAL,
		ROTATE_LEFT,
		ROTATE_RIGHT		
} ROTATE_FLAG;

typedef enum {
		NONE,
		USE_ITEM,
} ACTION_FLAG;

typedef struct {
		int item;			// 所有アイテム
		double dir;			// 進行方向
		VEROCITY ver;		// 速度ベクトル
		int alive;			// 生存フラグ
		BOOST_FLAG boost;	// 噴射フラグ
		ROTATE_FLAG rotate;	// 旋回フラグ
		ACTION_FLAG action;	// 行動フラグ
} PLAYER;

/* Range of objects */
typedef enum {
		RANGE_CHARACTER	= 20,
		RANGE_ITEM		= 15,
		RANGE_ROCK		= 10,
} OBJECT_RANGE;

// 通信データ(client > server)
typedef struct {
		int id;			// ユーザーID
		PLAYER player;	// プレイヤーのデータ
} entityState;

extern Charatype gChara[CT_NUM];
extern ITEM_NUMBER Item[ITEM_NUM];
extern PLAYER player;
extern POSITION positon;


#endif
