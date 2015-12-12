#include "../common.h"
#include "server_common.h"
#include "server_func.h"

#define sub(x)	x % RETENTION_FRAME

static ASSEMBLY pastAssembly[RETENTION_FRAME];
static ASSEMBLY *lastBuffer, *curBuffer;
static int frame;


static void setPlayerValue(PLAYER* to, PLAYER* from);


int initSystem() {
		frame = 0;
}


void updateBuffer() {
		lastBuffer = &pastAssembly[sub(frame)];
		curBuffer = &pastAssembly[sub(++frame)];
		*curBuffer = *lastBuffer;
}


void setPlayerState(int id, entityStateSet* state) {
		PLAYER* player = &curBuffer->player[id];
		player->object->pos.x = state->pos.x;
		player->object->pos.y = state->pos.y;
		setPlayerValue(player, &state->player);
}


static void setPlayerValue(PLAYER* to, PLAYER* from) {
		OBJECT* object = to->object;
		to = from;
		to->object = object;
}


void sendDeltaBuffer(int id, int latest) {
		entityStateGet data;
		ASSEMBLY *latestBuffer = &pastAssembly[sub(latest)];
		int i;

		/* デルタの所得 */
		for (i = 0; i < MAX_OBJECT; i++) {
				OBJECT* obj = &data.delta.object[i];
				OBJECT *curObj = curBuffer->object;
				OBJECT *latestObj = latestBuffer->object;
				obj->type = curObj->type - latestObj->type;
				obj->id = curObj->type - latestObj->type;
				obj->pos.x = curObj->pos.x - latestObj->pos.x;
				obj->pos.y = curObj->pos.y - latestObj->pos.y;
		}

		for (i = 0; i < MAX_CLIENTS; i++) {
				PLAYER* player = &data.delta.player[i];
				PLAYER *curPlayer = curBuffer->player;
				PLAYER* latestPlayer = latestBuffer->player;
				player->dir = curPlayer->dir - latestPlayer->dir;
				player->toDir = curPlayer->toDir - latestPlayer->toDir;
				player->ver.vx = curPlayer->ver.vx - latestPlayer->ver.vx;
				player->ver.vy = curPlayer->ver.vy - latestPlayer->ver.vy;
				player->alive = curPlayer->alive != latestPlayer->alive;
				player->boost = curPlayer->boost - latestPlayer->boost;
				player->rotate = curPlayer->rotate - latestPlayer->rotate;
				player->action = curPlayer->action - latestPlayer->action;
				player->item = curPlayer->item - latestPlayer->item;
				player->warn = curPlayer->warn - latestPlayer->warn;
				player->deadTime = curPlayer->deadTime - latestPlayer->deadTime;
				player->lastTime = curPlayer->lastTime - latestPlayer->lastTime;
		}

		sendData(id, &data, sizeof(entityStateGet));
}
