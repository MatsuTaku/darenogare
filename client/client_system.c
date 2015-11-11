#include "../common.h"
#include "client_common.h"
#include "client_func.h"

PLAYER myPlayer;

int initGameSystem(int myId, int playerNum) {
		myPlayer.item = 0;
		myPlayer.dir = 0;
		myPlayer.ver.vx = 0;
		myPlayer.ver.vy = 0;
		myPlayer.alive = true;

		return 0;
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
