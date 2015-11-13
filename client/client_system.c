#include "../common.h"
#include "client_common.h"
#include "client_func.h"

#define nextObj(a)	(a++ % MAX_OBJECT)

OBJECT allObject[MAX_OBJECT];
PLAYER allPlayer[MAX_CLIENTS];
PLAYER* myPlayer;
int myPlayerSub;

int curObject;

static int insertObject(void* buffer, OBJECT_TYPE type);
static bool hitObject(OBJECT* alpha, OBJECT* beta);
static double getObjectSize(OBJECT* object);
static double getRange(OBJECT* alpha, OBJECT* beta);

int initGameSystem(int myId, int playerNum) {
		int i;

		for (i = 0; i < MAX_OBJECT; i++) {
				OBJECT* object = &allObject[i];
				object->type = EMPTY;
				object->typeBuffer = NULL;
		}
		curObject = 0;

		myPlayer = &allPlayer[myId];

		for (i = 0; i < playerNum; i++) {
				PLAYER *player = &allPlayer[i];
				player->item = 0;
				player->dir = 0;
				player->ver.vx = 0;
				player->ver.vy = 0;
				player->alive = true;
				if ((myPlayerSub = insertObject(player, CHARACTER)) == -1) {
						fprintf(stderr, "insert object is failed!\n");
						return-1;
				}
		}

		return 0;
}

static int insertObject(void* buffer, OBJECT_TYPE type) {
		int count = 0;
		while (count < MAX_OBJECT) {
				OBJECT* object = &allObject[curObject];
				if (object->type == EMPTY) {
						object->type = type;
						object->typeBuffer = buffer;
						return curObject;
				}
				curObject = nextObj(curObject);
				count++;
		}
		return -1;
}

/*
 *	触れたアイテムを入手する
 */
void getItem() {
		int num, i;
		OBJECT* playerObj = &allObject[myPlayerSub];
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
		// MARK
		if (myPlayer->item != ITEM_EMPTY) {
				switch (myPlayer->item) {	// アイテムごとに決められた処理を行う
						case ITEM_THUNDER:
								break;
						case ITEM_LASER:
								break;
						case ITEM_TRAP:
								break;
						case ITEM_MINIMUM:
								break;
						default:
								break;
				}
				myPlayer->item = ITEM_EMPTY;	// 使用後消去
		}
}


/*
 * 方向転換
 * 引数　：　アナログスティックを倒した方向
 */
void changeDir(int dir) {
		// MARK
		switch (dir) {
				case LEFT:
						break;
				case RIGHT:
						break;
				case UP:
						break;
				case DOWN:
						break;
				default:
						break;
		}
}


/*
 * 機体の速度変更
 */
void boost(int dir) {
		// MARK
		switch (dir) {
				case GO:
						break;
				case BACK:
						break;
				case NEUTRAL:
						break;
				default:
						break;
		}
}


/*
 *	当たり判定
 *	input:	オブジェクトポインタ
 */
static bool hitObject(OBJECT* alpha, OBJECT* beta) {
		double rimitRange = getObjectSize(alpha) + getObjectSize(beta);
		return getRange(alpha, beta) <= pow(rimitRange, 2);
}


/*
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


/*
 *	距離の所得
 *	input:	オブジェクトポインタ
 *	return: 距離の2乗
 */
static double getRange(OBJECT* alpha, OBJECT* beta) {
		 return pow(alpha->pos.x - beta->pos.x, 2) + pow(alpha->pos.y - beta->pos.y, 2);
}
