#include "../common.h"
#include "client_common.h"
#include "client_func.h"

#define nextObj(a)	((a + 1) % MAX_OBJECT)

ASSEMBLY allAssembly;
OBJECT* object;
PLAYER* player;
OBSTACLE* obstacle;
PLAYER* myPlayer;

int curObjNum;

static void initPlayer(PLAYER* player);
static OBJECT* insertObject(void* buffer, OBJECT_TYPE type);
static void rotateDirection(double sign);
static void accelerateVerocity(double accel);
static void setPlayerPosition();
static bool hitObject(OBJECT* alpha, OBJECT* beta);
static double getObjectSize(OBJECT* object);
static double getRange(OBJECT* alpha, OBJECT* beta);


/**
 *	ゲームシステムの初期化
 *	return: Error = -1
 */
int initGameSystem(int myId, int playerNum) {
		int i;
		object = allAssembly.object;
		player = allAssembly.player;
		obstacle = allAssembly.obstacle;

		for (i = 0; i < MAX_OBJECT; i++) {
				OBJECT* curObj = &object[i];
				curObj->type = OBJECT_EMPTY;
		}
		curObjNum = 0;

		myPlayer = &player[myId];

		for (i = 0; i < playerNum; i++) {
				PLAYER* curPlayer = &player[i];
				if (insertObject(curPlayer, OBJECT_CHARACTER) == NULL) {
						fprintf(stderr, "Inserting OBJECT is failed!\n");
						return -1;
				}
				curPlayer->num = i;
				initPlayer(curPlayer);
		}

		return 0;
}


static void initPlayer(PLAYER* player) {
		player->item = ITEM_EMPTY;
		player->dir = PI / 2;
		player->toDir = player->dir;
		player->ver.vx = 0;
		player->ver.vy = 0;
		player->alive = true;
		player->object->pos.x = 640;
		player->object->pos.y = 360;
}


/**
 * オブジェクトの挿入
 * input1: オブジェクト内容バッファ
 * input2: オブジェクトタイプ
 * return: オブジェクトのポインタ(error = NULL)
 */
static OBJECT* insertObject(void* buffer, OBJECT_TYPE type) {
		int count = 0;
		OBJECT* curObject = NULL;

		while (count < MAX_OBJECT) {
				curObject = &object[curObjNum];
				if (object->type == OBJECT_EMPTY) {
						curObject->type = type;
						curObject->typeBuffer = buffer;
						switch (type) {
								case OBJECT_EMPTY:
										break;
								case OBJECT_CHARACTER:
										((PLAYER *)buffer)->object = object;
										break;
								case OBJECT_ITEM:
										((ITEM *)buffer)->object = object;
										break;
								case OBJECT_OBSTACLE:
										((OBSTACLE *)buffer)->object = object;
										break;
								default:
										break;
						}
						curObjNum = nextObj(curObjNum);
						return curObject;
				}
				curObjNum = nextObj(curObjNum);
				count++;
		}

		return NULL;
}


void updateEvent() {
		/** Player value change method */
		/* プレイヤーの行動 */
		// MARK
		switch (myPlayer->action) {
				case NONE:	break;
				case USE_ITEM:
							break;
				default:
							break;
		}

		/* 旋回 */
		switch (myPlayer->rotate) {
				case ROTATE_NEUTRAL:	
						break;
				case ROTATE_LEFT:
						rotateDirection(1);
						break;
				case ROTATE_RIGHT:
						rotateDirection(-1);
						break;
				default:
						break;
		}

		/* 加減速 */
		switch (myPlayer->boost) {
				case BOOST_NEUTRAL:	
						break;
				case BOOST_GO:
						accelerateVerocity(ACCELE_GO);
						break;
				case BOOST_BACK:
						accelerateVerocity(ACCELE_BRAKE);
						break;
				default:
						break;
		}

		/* 速度を元に座標移動 */
		setPlayerPosition();
}


/**
 * 機体を旋回
 * input: 単位角速度
 */
static void rotateDirection(double sign) {
		double toDir = myPlayer->dir + sign * (((double)ANGULAR_VEROCITY * PI / HALF_DEGRESS) / FPS);
		double da = myPlayer->dir - myPlayer->toDir;
		double db = toDir - myPlayer->toDir;
		if ((da * db) < 0 || abs(db) > 2 * PI) {	// 回転しすぎた場合
				toDir = myPlayer->toDir;
				printf("fix\n");
		}
		// -PI ~ PI 
		while (toDir > PI) 
				toDir -= 2 * PI;
		while (toDir <= -PI) 
				toDir += 2 * PI;
		myPlayer->dir = toDir;
#ifndef NDEBUG
		// printf("player direction[%3.0f°]\n", myPlayer->dir * HALF_DEGRESS / PI);
#endif	
}


/**
 * 噴射と向きに基づく速度変更
 * input: 機体の向きに対する加速度
 */
static void accelerateVerocity(double accel) {
		double direction = myPlayer->dir;
		myPlayer->ver.vx += accel * cos(direction) / FPS;
		myPlayer->ver.vy += accel * -sin(direction) / FPS;
#ifndef NDEBUG
		double v = sqrt(pow(myPlayer->ver.vx, 2) + pow(myPlayer->ver.vy, 2));
		printf("plyaer verocity[|V|: %4.0f, vx: %4.0f, vy: %4.0f]\n", v, myPlayer->ver.vx, myPlayer->ver.vy);
#endif
}


/**
 * 機体の向きと速度から座標を移動
 */
static void setPlayerPosition() {
		POSITION* pos = &(myPlayer->object->pos);
		pos->x += myPlayer->ver.vx;
		pos->y += myPlayer->ver.vy;
#ifndef NDEBUG
		// printf("player pos[x: %4d, y: %4d]\n", pos->x, pos->y);
#endif
}


/*
 *	触れたアイテムを入手する
 */
void getItem() {
		int num, i;
		OBJECT* playerObj = myPlayer->object;

		for (i = 0; i < MAX_OBJECT; i++) {
				OBJECT* curObj = &object[i];
				if (curObj->type == OBJECT_ITEM) {
						if (hitObject(playerObj, curObj)) {
								// MARK
								myPlayer->item = curObj->id;
								break;
						}
				}
		}
}


void useItem() {	// アイテムの使用
		if (myPlayer->item != ITEM_EMPTY) {
				myPlayer->action = USE_ITEM;
		}
}


/**
 * 方向転換
 */
void rotateTo(int x, int y) {
		double range = sqrt(pow(x, 2) + pow(y, 2));
		double px = (double)x / range;
		double py = -(double)y / range;
		double toAngle = acos(px) * ((py >= 0) ? 1 : -1);
		if (toAngle > PI)	toAngle -= 2 * PI;
		myPlayer->toDir = toAngle;
#ifndef NDEBUG
		// printf("toAngle: %3.0f\n", toAngle * HALF_DEGRESS / PI);
#endif
		double dAngle = toAngle - myPlayer->dir;
		if (dAngle == 0)
				fixRotation();
		else if (dAngle <= -PI || (0 < dAngle && dAngle <= PI))
				rotateLeft();
		else
				rotateRight();
}


void rotateLeft() {
		myPlayer->rotate = ROTATE_LEFT;
}

void rotateRight() {
		myPlayer->rotate = ROTATE_RIGHT;
}

void fixRotation() {
		myPlayer->rotate = ROTATE_NEUTRAL;
}


/**
 * 機体の速度変更
 */
void acceleration() {
		myPlayer->boost = BOOST_GO;
#ifndef NDEBUG
		printf("Acceleration.\n");
#endif
}

void deceleration() {
		myPlayer->boost = BOOST_BACK;
#ifndef NDEBUG
		printf("Deceleration.\n");
#endif
}

void inertialNavigation() {
		myPlayer->boost = BOOST_NEUTRAL;
#ifndef NDEBUG
		printf("Inertial navigation.\n");
#endif
}


/**
 *	当たり判定
 *	input:	オブジェクトポインタ
 *	return:	判定
 */
static bool hitObject(OBJECT* alpha, OBJECT* beta) {
		double rimitRange = getObjectSize(alpha) + getObjectSize(beta);
		return getRange(alpha, beta) <= pow(rimitRange, 2);
}


/**
 *	オブジェクト半径を所得
 *	input:	オブジェクトポインタ
 *	return: オブジェクトの半径(double)
 */
static double getObjectSize(OBJECT* object) {
		double size = 0;
		switch (object->type) {
				case OBJECT_CHARACTER:
						size = RANGE_CHARACTER;
						break;
				case OBJECT_ITEM:
						size = RANGE_ITEM;
						break;
				case OBJECT_OBSTACLE:
						size = RANGE_ROCK;
						break;
				case OBJECT_EMPTY:
				default:
						break;
		}
		return size;
}


/**
 *	距離の所得
 *	input:	オブジェクトポインタ
 *	return: 距離の2乗
 */
static double getRange(OBJECT* alpha, OBJECT* beta) {
		return pow(alpha->pos.x - beta->pos.x, 2) + pow(alpha->pos.y - beta->pos.y, 2);
}
