#include "../common.h"
#include "server_common.h"
#include "server_func.h"
#define nextObj(a)	((a + 1) % MAX_OBJECT)

#define sub(x)	x % RETENTION_FRAME

static ASSEMBLY pastAssembly[RETENTION_FRAME];
static ASSEMBLY *lastBuffer, *curBuffer;
static int frame;
static int clientNum;
static int curObjNum;

static void initObject(OBJECT* object);
static setPos(POSITION* pos, int x, int y);
static OBJECT* insertObject(void* buffer, OBJECT_TYPE type);

static void setPlayerValue(PLAYER* to, PLAYER* from);


int initSystem(int clientNumber) {
		frame = 0;
		srand((unsigned)time(NULL));
		clientNum = clientNumber;
		int i;

		ASSEMBLY* firstData = &pastAssembly[sub(frame)];
		lastBuffer = firstData;
		curBuffer = firstData;

		for (i = 0; i < MAX_OBJECT; i++) {
				initObject(&firstData->object[i]);
		}
		curObjNum = 0;

		for (i = 0; i < clientNumber; i++) {
				if (insertObject(&firstData->player[i], OBJECT_CHARACTER) == NULL) {
						fprintf(stderr, "InsertingObject is failed!\n");
						return -1;
				}
		}
}

static void initObject(OBJECT* object) {
		object->type = OBJECT_EMPTY;
		object->id = 0;
		setPos(&object->pos, 0, 0);
}

static OBJECT* insertObject(void* buffer, OBJECT_TYPE type) {
		assert(buffer != NULL);
		assert(type >= 0);
		assert(type < OBJECT_NUM);
		int count = 0;
		while (count < MAX_OBJECT) {
				OBJECT* curObject = &curBuffer->object[curObjNum];
				if (curObject->type == OBJECT_EMPTY) {
						curObject->type = type;
						curObject->typeBuffer = buffer;
						switch (type) {
								case OBJECT_EMPTY:
										break;
								case OBJECT_CHARACTER:
										((PLAYER *)buffer)->object = curObject;
										break;
								case OBJECT_ITEM:
										((ITEM *)buffer)->object = curObject;
										break;
								case OBJECT_OBSTACLE:
										((OBSTACLE *)buffer)->object = curObject;
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

static setPos(POSITION* pos, int x, int y) {
		pos->x = x;
		pos->y = y;
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


void sendDeltaBuffer(int id, int latest, bool endFlag) {
		entityStateGet data;
		ASSEMBLY *latestBuffer = &pastAssembly[sub(latest)];

		data.endFlag = endFlag;

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
