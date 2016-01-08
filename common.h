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
#define CPS 20
#define MIRI_SECOND		1000
#define RETENTION_FRAME	0x7f

#define MAX_CLIENTS		0x04				
#define MAX_NAME_SIZE	10 				

#define MAX_DATA		200				
#define MAX_OBJECT		0xff
#define MAX_OBSTACLE	0x6f
#define MAX_ITEM		0x3f
#define CT_NUM 4

#define MAX_EVENT	0x0f
#define MAP_SIZE	10000
#define WORLD_SIZE 		20000

#define PI				3.14159265
#define HALF_DEGRESS	180
#define MAXIMUM_SPEED_OBSTACLE 200

typedef enum {
		END_COMMAND = 1
} COMMANDS;

// オブジェクトタイプ列挙体
typedef enum {
		OBJECT_EMPTY,
		OBJECT_CHARACTER,
		OBJECT_ITEM,
		OBJECT_OBSTACLE,
		OBJECT_NUM
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
		ITEM_EMPTY      = -1,
		ITEM_NOIZING	= 0,
		ITEM_LASER	= 1,
		ITEM_MISSILE	= 2,
		ITEM_MINIMUM	= 3,
		ITEM_BARRIER	= 4,
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
		OWNER = -1,
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
		ACTION_NONE,
		ACTION_USE_ITEM,
} ACTION_FLAG;

typedef enum {
		WARN_SAFETY,
		WARN_OUT_AREA
} WARNING_FLAG;

typedef struct {
		OBJECT* object;		// 固有オブジェクトバッファ
		PLAYER_NUMBER num;	// プレイヤー番号
		double dir;			// 進行方向
		double toDir;		// 目標旋回角度
		VEROCITY ver;		// 速度ベクトル
		bool alive;			// 生存フラグ
		BOOST_FLAG boost;	// 噴射フラグ
		ROTATE_FLAG rotate;	// 旋回フラグ
		ACTION_FLAG action;	// 行動フラグ
		int item;			// 所有アイテム
		WARNING_FLAG warn;	// 警告フラグ
		int deadTime;		// 死亡時間(生存領域外にいるとき)
		int lastTime;		// 生存可能時間（〃）
		int deadAnimation;
} PLAYER;


// すべてのオブジェクトの集合体
typedef struct {
		OBJECT object[MAX_OBJECT];
		PLAYER player[MAX_CLIENTS];
} ASSEMBLY;


// イベント通知
typedef enum {
		EVENT_NONE = -1,
		EVENT_OBSTACLE = 0,
		EVENT_ITEM = 1,
		EVENT_KILL = 2,
} EVENT;

typedef struct {
		int playerId;	// プレイヤー番号
		EVENT type;	// イベントの種類
		int objId;		// オブジェクトの種類
		int id;			// オブジェクトID
		POSITION pos;
		double angle;
		double ver;
		int killTo;
} eventNotification;


// 通信データ(client -> server)
typedef struct {
		int latestFrame;// 受信した最新フレーム 
		bool endFlag;	// 終了フラグ
		int clientId;	// ユーザーID
		POSITION pos;	// プレイヤーポジション
		PLAYER player;	// プレイヤーのデータ
		eventNotification event[MAX_EVENT];
} entityStateSet;


// 通信データ(server -> client)
typedef struct {
		OBJECT plyObj[MAX_CLIENTS];
		PLAYER player[MAX_CLIENTS];
} DELTA;

typedef struct {
		int latestFrame;
		int lastFrame;
		bool endFlag;
		DELTA delta;
		eventNotification event[MAX_EVENT];
} entityStateGet;

#endif
