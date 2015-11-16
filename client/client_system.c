#include "../common.h"
#include "client_common.h"
#include "client_func.h"

#define nextObj(a)	(a++ % MAX_OBJECT)

OBJECT allObject[MAX_OBJECT];
PLAYER allPlayer[MAX_CLIENTS];
PLAYER* myPlayer;

int curObject;

static int insertObject(void* buffer, OBJECT_TYPE type);
static void rotateDirection(double sign);
static void setPlayerPosition();
static bool hitObject(OBJECT* alpha, OBJECT* beta);
static double getObjectSize(OBJECT* object);
static double getRange(OBJECT* alpha, OBJECT* beta);


/*
 *	ゲームシステムの初期化
 *	return: Error = -1
 */
int initGameSystem(int myId, int playerNum) {
		int i;

		for (i = 0; i < MAX_OBJECT; i++) {
				OBJECT* object = &allObject[i];
				object->type = EMPTY;
				object->typeBuffer = NULL;
				object->pos.x = 0;
				object->pos.y = 0;
		}
		curObject = 0;

		myPlayer = &allPlayer[myId];

		for (i = 0; i < playerNum; i++) {
				PLAYER *player = &allPlayer[i];
				player->item = 0;
				player->dir = 0;
				// player->ver.vx = 0;
				// player->ver.vy = 0;
				player->ver = 0;
				player->alive = true;
				if (insertObject(player, CHARACTER) == NULL) {
						fprintf(stderr, "Inserting OBJECT is failed!\n");
						return -1;
				}
		}

		return 0;
}


/**
 * オブジェクトの挿入
 * input1: オブジェクト内容バッファ
 * input2: オブジェクトタイプ
 * return: オブジェクトのポインタ(error = NULL)
 */
static OBJECT* insertObject(void* buffer, OBJECT_TYPE type) {
		int count = 0;
		OBJECT* object = NULL;

		while (count < MAX_OBJECT) {
				object = &allObject[curObject];
				if (object->type == EMPTY) {
						object->type = type;
						object->typeBuffer = buffer;
						switch (type) {
								case EMPTY:
										break;
								case CHARACTER:
										((PLAYER *)buffer)->object = object;
										break;
								case ITEM:
										((ITEM *)buffer)->object = object;
										break;
								case OBSTACLE:
										((OBSTACLE *)buffer)->object = object;
										break;
								default:
										break;
						}
						curObject = nextObj(curObject);
						return object;
				}
				curObject = nextObj(curObject);
				count++;
		}

		return object;
}


void updateEvent() {
		/** Player value change */

		/* プレイヤーの行動 */
		// MARK
		switch (myPlayer->action) {
				case NONE:
						break;
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
						myPlayer->ver += RESISTANCE;
						break;
				case BOOST_GO:
						myPlayer->ver += ACCELE_GO;
						break;
				case BOOST_BACK:
						myPlayer->ver += ACCELE_BRAKE;
						break;
				default:
						break;
		}

		/* 角度と速度を元に座標移動 */
		/*
		POSITION* myPos = &(myPlayer->object->pos);
		myPos->x += myPlayer->ver.vx / FPS;
		myPos->y += myPlayer->ver.vy / FPS;
		*/
		setPlayerPosition();
}


/**
 * 機体を旋回
 * input: 回転方向単位値
 */
static void rotateDirection(double sign) {
		double toDir = myPlayer->dir + sign * (ANGULAR_VEROCITY / FPS / HALF_DEGRESS * PI);
		while (toDir > PI) {
				toDir -= 2 * PI;
		}
		while (toDir < -PI) {
				toDir += 2 * PI;
		}
		myPlayer->dir = toDir;
}


/**
 * 機体の向きと速度から座標を移動
 */
static void setPlayerPosition() {
		double verocity = myPlayer->ver;
		double angle = myPlayer->dir;

		POSITION* pos = &(myPlayer->object->pos);
		pos->x += verocity * cos(angle) / FPS;
		pos->y += verocity * sin(angle) / FPS;
}


/*
 *	触れたアイテムを入手する
 */
void getItem() {
		int num, i;
		OBJECT* playerObj = myPlayer->object;
		for (i = 0; i < MAX_OBJECT; i++) {
				OBJECT* curObj = &allObject[i];
				if (curObj->type == ITEM) {
						if (hitObject(playerObj, curObj)) {
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


/*
 * 方向転換
 */

void rotateLeft() {
		myPlayer->rotate = ROTATE_LEFT;
}

void rotateRight() {
		myPlayer->rotate = ROTATE_RIGHT;
}

void fixRotation() {
		myPlayer->rotate = ROTATE_NEUTRAL;
}


/*
 * 機体の速度変更
 */
void acceleration() {
		myPlayer->boost = BOOST_GO;
}

void deceleration() {
		myPlayer->boost = BOOST_BACK;
}

void inertialNavigation() {
		myPlayer->boost = BOOST_NEUTRAL;
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
				case CHARACTER:
						size = RANGE_CHARACTER;
						break;
				case ITEM:
						size = RANGE_ITEM;
						break;
				case OBSTACLE:
						size = RANGE_ROCK;
						break;
				case EMPTY:
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
