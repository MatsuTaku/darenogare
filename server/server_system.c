#include "../common.h"
#include "server_common.h"
#include "server_func.h"
#define nextObj(a)	((a + 1) % MAX_OBJECT) + MAX_CLIENTS

#define sub(x)	x % RETENTION_FRAME

static ASSEMBLY pastAssembly[RETENTION_FRAME];
static ASSEMBLY *lastBuffer, *curBuffer;
static eventAssembly pastEvent[RETENTION_FRAME];
static eventAssembly *lastEvent, *curEvent;
static int frame;
static int clientNum;
static int curObjNum;
static int clientNum;

static void initObject(OBJECT* object);
static bool generateObstacle(int id, int num, POSITION* pos, double angle, double ver);
static setPos(POSITION* pos, int x, int y);
static OBJECT* insertObject(void* buffer, OBJECT_TYPE type);

static bool insertEvent(int id, eventNotification* event);
static void setPlayerValue(PLAYER* to, PLAYER* from);


int initSystem(int clientNumber) {
		frame = 0;
		clientNum = clientNumber;
		srand((unsigned)time(NULL));
		clientNum = clientNumber;
		int i;

		ASSEMBLY* firstData = &pastAssembly[sub(frame)];
		lastBuffer = firstData;
		curBuffer = firstData;
		eventAssembly* firstEvent = &pastEvent[sub(frame)];
		lastEvent = firstEvent;
		curEvent = firstEvent;

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
		curObjNum = MAX_CLIENTS;
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

static bool generateObstacle(int id, int num, POSITION* pos, double angle, double ver) {
		OBSTACLE* curObs;
		if ((curObs = malloc(sizeof(OBSTACLE))) == NULL) {
				fprintf(stderr, "Out of memory! Failed to insert obstacle.\n");
				exit(-1);
		}
		if (insertObject(curObs, OBJECT_OBSTACLE) == NULL) {
				fprintf(stderr, "Failed to insert object\n");
				return false;
		}
		curObs->object->pos.x = pos->x;
		curObs->object->pos.y = pos->y;
		curObs->object->id = id;
		curObs->angle = angle;
		curObs->ver = ver;
}

static setPos(POSITION* pos, int x, int y) {
		pos->x = x;
		pos->y = y;
}


void updateBuffer() {
		lastBuffer = &pastAssembly[sub(frame)];
		curBuffer = &pastAssembly[sub(frame + 1)];
		*curBuffer = *lastBuffer;
		lastEvent = &pastEvent[sub(frame)];
		curEvent = &pastEvent[sub(frame + 1)];
		*curEvent = *lastEvent;
		frame++;
}


static bool insertEvent(int id, eventNotification* event) {
		int i;
		for (i = 0; i < clientNum; i++) {
				if (i != id) {
						int j;
						for (j = 0; j < MAX_EVENT; j++)
								if (curEvent->eventStack[i][j].id == EVENT_NONE)
										curEvent->eventStack[i][j] = *event;
						if (j == MAX_EVENT) {
								printf("player[%d] Event stack is FULL!\n", id);
								return false;
						}
				}
		}
		return true;
}


static clearEvent(int playerId, int latest) {
		int nowFrame = frame;
		int i;
		for (i = sub(latest); 
		i != sub(nowFrame); 
		i = i > 0 ? i - 1 : RETENTION_FRAME - 1) {
				int j;
				for (j = 0; j < MAX_EVENT; j++) {
						eventNotification new;
						new.type = EVENT_NONE;
						pastEvent[i].eventStack[playerId][j] = new;
				}

		}
}


void setPlayerState(int id, entityStateSet* state) {
		PLAYER* player = &curBuffer->player[id];
		OBJECT *plyObj = &curBuffer->object[id];
		plyObj->pos.x = state->pos.x;
		plyObj->pos.y = state->pos.y;
		printf("frame[%d]	player[%d] pos x: %d, y: %d\n", frame, id, plyObj->pos.x, plyObj->pos.y);
		printf("				alive: %d\n", state->player.alive);
		printf("				alive: %d\n", player->alive);
		setPlayerValue(player, &state->player);
		int i;
		for (i = 0; i < MAX_EVENT; i++) {
				if (state->event[i].type != EVENT_NONE) {
						if (!insertEvent(id, &state->event[i]))
								exit(-1);
				}
		}
}


static void setPlayerValue(PLAYER* to, PLAYER* from) {
		OBJECT* object = to->object;
		*to = *from;
		to->object = object;
}


void sendDeltaBuffer(int id, int latest, bool endFlag) {
		entityStateGet data;

		if ((data.endFlag = endFlag) == false) {
				assert(id >= 0 && id < MAX_CLIENTS);
				assert(latest >= 0);
				ASSEMBLY *latestBuffer = &pastAssembly[sub(latest)];
				int i;
				data.latestFrame = frame;
				data.lastFrame = latest;
				/* デルタの所得 */
				for (i = 0; i < clientNum; i++) {
						PLAYER* player = &data.delta.player[i];
						PLAYER *curPlayer = &curBuffer->player[i];
						PLAYER* latestPlayer = &latestBuffer->player[i];
						player->dir = curPlayer->dir - latestPlayer->dir;
						player->toDir = curPlayer->toDir - latestPlayer->toDir;
						player->ver.vx = curPlayer->ver.vx - latestPlayer->ver.vx;
						player->ver.vy = curPlayer->ver.vy - latestPlayer->ver.vy;
						player->alive = curPlayer->alive;	// bool値は直接送信
						player->boost = curPlayer->boost - latestPlayer->boost;
						player->rotate = curPlayer->rotate - latestPlayer->rotate;
						player->action = curPlayer->action - latestPlayer->action;
						player->item = curPlayer->item - latestPlayer->item;
						player->warn = curPlayer->warn - latestPlayer->warn;
						player->deadTime = curPlayer->deadTime - latestPlayer->deadTime;
						player->lastTime = curPlayer->lastTime - latestPlayer->lastTime;

						OBJECT *object = &data.delta.plyObj[i];
						OBJECT *curPlObj = &curBuffer->object[i];
						OBJECT *latestPlObj = &latestBuffer->object[i];
						object->type = curPlObj->type - latestPlObj->type;
						object->id = curPlObj->id - latestPlObj->id;
						object->pos.x = curPlObj->pos.x - latestPlObj->pos.x;
						object->pos.y = curPlObj->pos.y - latestPlObj->pos.y;
						
						printf("frame[%d]: latest[%d]	player[%d] pos x: %d, y: %d\n", frame, latest, i, object->pos.x, object->pos.y);
						printf("				alive: %d\n", player->alive);
				}

				/* イベント所得 */
				for (i = 0; i < MAX_EVENT; i++) {
						if (curEvent->eventStack[id][i].id != EVENT_NONE && 
							curEvent->eventStack[id][i].id != lastEvent->eventStack[id][i].id) {
								data.event[i] = curEvent->eventStack[id][i];
						} else {
								data.event[i].type = EVENT_NONE;
						}
				}
				clearEvent(id, latest);

				printf("send server frame: %d\n", data.latestFrame);
		}

		sendData(id, &data, sizeof(entityStateGet));
}
