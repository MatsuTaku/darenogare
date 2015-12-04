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
#define RETENTION_FRAME	10

#define MAX_CLIENTS		0x04				
#define MAX_NAME_SIZE	10 				

#define MAX_DATA		200				
#define MAX_OBJECT		0xff
#define MAX_OBSTACLE	0x6f
#define MAX_ITEM		0x3f
#define CT_NUM 4

#define MAP_SIZE	10000
#define WORLD_SIZE 		20000

typedef enum {
		END_COMMAND = 1
} COMMANDS;

// オブジェクトタイプ列挙体
typedef enum {
		OBJECT_EMPTY,
		OBJECT_CHARACTER,
		OBJECT_ITEM,
		OBJECT_OBSTACLE
} OBJECT_TYPE;


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


typedef struct {
		double vx;	// 速度成分X
		double vy;	// 速度成分Y
} VEROCITY;


/* OBSTACLE Values */
typedef struct {
		OBJECT* object;
		double angle;
		double ver;
} OBSTACLE;


/* ITEM Values */
typedef enum {
		ITEM_EMPTY      = 0,
		ITEM_NOIZING	= 1,
		ITEM_LASER	= 2,
		ITEM_MISSILE	= 3,
		ITEM_MINIMUM	= 4,
		ITEM_BARRIER	= 5,
		ITEM_NUM	= 5
} ITEM_NUMBER ;

typedef struct {
		// MARK
		OBJECT* object;
		ITEM_NUMBER num;
} ITEM;


/* Range of objects */
typedef enum {
		RANGE_CHARACTER = 40,
		RANGE_ITEM = 25,
		RANGE_ROCK = 15,
} OBJECT_RANGE;


/* PLAYER Values */
typedef enum {
		PLAYER_1 = 0,
		PLAYER_2 = 1,
		PLAYER_3 = 2,
		PLAYER_4 = 3,
		PLAYER_MAX = 4
} PLAYER_NUMBER;

typedef enum {
		BOOST_NEUTRAL,
		BOOST_GO,
		BOOST_BACK
} BOOST_FLAG;

typedef enum {
		ROTATE_RIGHT = -1,
		ROTATE_NEUTRAL = 0,
		ROTATE_LEFT = 1,
} ROTATE_FLAG;

typedef enum {
		NONE,
		USE_ITEM,
} ACTION_FLAG;

typedef struct {
		OBJECT* object;		// 固有オブジェクトバッファ
		PLAYER_NUMBER num;	// プレイヤー番号
		double dir;			// 進行方向
		double toDir;		// 目標旋回角度
		VEROCITY ver;		// 速度ベクトル
		int alive;			// 生存フラグ
		BOOST_FLAG boost;	// 噴射フラグ
		ROTATE_FLAG rotate;	// 旋回フラグ
		ACTION_FLAG action;	// 行動フラグ
		int item;			// 所有アイテム
		int id;
} PLAYER;



// すべてのオブジェクトの集合体
typedef struct {
		OBJECT object[MAX_OBJECT];
		PLAYER player[MAX_CLIENTS];
		OBSTACLE obstacle[MAX_OBSTACLE];
		ITEM item[MAX_ITEM];
} ASSEMBLY;


// 通信データ(client -> server)
typedef struct {
		int endFlag;	// 終了フラグ
		int clientId;			// ユーザーID
		POSITION pos;	// プレイヤーポジション
		PLAYER player;	// プレイヤーのデータ
} entityStateSet;


// 通信データ(server -> client)
typedef struct {
		int endFlag;
		int clientId;
		POSITION pos;
		PLAYER player;
		int killEnemy;
		int deadFlag;
} entityStateGet;


typedef struct {
		int test;
} mapData;


#endif
