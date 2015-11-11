#include "../common.h"
#include "client_common.h"
#include "client_func.h"

#define nextObj(a)	(a++ % MAX_OBJECT)

PLAYER allPlayer[MAX_CLIENTS];
PLAYER* myPlayer;
OBJECT allObject[MAX_OBJECT];

int curObject;

static int insertObject(void* buffer);

int initGameSystem(int myId, int playerNum) {
		int i;

		for (i = 0; i < MAX_OBJECT; i++) {
				OBJECT* object = allObject[i];
				object.type = EMPTY;
				object.typeBuffer = NULL;
		}
		curObject = 0;

		myPlayer = &(allPlayer[myId]);

		for (i = 0; i < playerNum; i++) {
				PLAYER *player = allPlayer[i];
				player.item = 0;
				player.dir = 0;
				player.ver.vx = 0;
				player.ver.vy = 0;
				player.alive = true;
				if (insertObject(player) == -1) {
						fprintf(stderr, "insert object is failed!\n");
						return-1;
				}
		}

		return 0;
}

static int insertObject(void* buffer) {
		int count = 0;
		while (count < MAX_OBJECT) {
				OBJECT* object = allObject[curObject];
				if (object.type == EMPTY) {
						object.type = CHARACTER;
						object.typeBuffer = buffer;
						return 0;
				}
				curObject = nextOBj(curObject);
				count++;
		}
		return -1;
}

void useItem() {	// アイテムの使用
		// MARK
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
