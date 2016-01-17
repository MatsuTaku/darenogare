#include "../common.h"
#include "server_common.h"
#include "server_func.h"
#define nextObj(a)	((a + 1) % MAX_OBJECT) + MAX_CLIENTS

#define sub(x)	(x) % RETENTION_FRAME

static ASSEMBLY pastAssembly[RETENTION_FRAME];
static ASSEMBLY *lastBuffer, *curBuffer;
static eventAssembly pastEvent[RETENTION_FRAME];
static eventAssembly *lastEvent, *curEvent;
static int frame;
static int clientNum;
static int curObjNum;
static int ownerObject;

static void initObject(OBJECT* object);
static void initPlayer(PLAYER* player, int id);
static bool generateObstacle(int id, int num, POSITION* pos, double angle, double ver);
static setPos(POSITION* pos, int x, int y);
static void initEvent(eventNotification* event);
static bool averageFromFrequency(double freq);
static bool randomGenerateObstacle();
static bool randomGenerateItem();
static bool insertEvent(int id, eventNotification* event);
static OBJECT* insertObject(void* buffer, int id, OBJECT_TYPE type);
static void setPlayerValue(PLAYER* to, PLAYER* from);


int initSystem(int clientNumber) {
		srand((unsigned)time(NULL));

		frame = 0;
		clientNum = clientNumber;
		int i, j;

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
		ownerObject = 0x0000;

		for (i = 0; i < MAX_CLIENTS; i++) {
				for (j = 0; j < MAX_EVENT; j++) {
						initEvent(&firstEvent->eventStack[i][j]);
				}
		}

		for (i = 0; i < clientNumber; i++) {
				if (insertObject(&firstData->player[i], 0x0100 * i, OBJECT_CHARACTER) == NULL) {
						fprintf(stderr, "InsertingObject is failed!\n");
						return -1;
				}
				initPlayer(&firstData->player[i], i);
		}
		curObjNum = MAX_CLIENTS;
#ifndef NDEBUG
		printf("###initSystem\n");
#endif
}

static void initObject(OBJECT* object) {
		object->type = OBJECT_EMPTY;
		object->id = 0;
		setPos(&object->pos, 0, 0);
}

static void initPlayer(PLAYER* player, int id) {
		player->object = NULL;
		player->num = id;
		player->dir = 0;
		player->toDir = player->dir;
		player->ver.vx = 0;
		player->ver.vy = 0;
		player->alive = 0;
		player->boost = 0;
		player->rotate = 0;
		player->action = 0;
		player->item = 0;
		player->launchCount = 0;
		player->warn = 0;
		player->deadTime = 0;
		player->lastTime = 0;
		player->deadAnimation = 0;
}

static OBJECT* insertObject(void* buffer, int id, OBJECT_TYPE type) {
		assert(buffer != NULL);
		assert(type >= 0);
		assert(type < OBJECT_NUM);
		int count = 0;
		while (count < MAX_OBJECT) {
				OBJECT* curObject = &curBuffer->object[curObjNum];
				if (curObject->type == OBJECT_EMPTY) {
						curObject->id = id;
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
		if (insertObject(curObs, id, OBJECT_OBSTACLE) == NULL) {
				fprintf(stderr, "Failed to insert object\n");
				return false;
		}
		curObs->object->pos = *pos;
		curObs->object->id = id;
		curObs->num = num;
		curObs->angle = angle;
		curObs->ver = ver;
		return true;
}

static bool insertItem(int id, ITEM_NUMBER num, POSITION* pos) {
		ITEM* item;
		if ((item = malloc(sizeof(ITEM))) == NULL) {
				fprintf(stderr, "Out of memory! Failed to insert item.\n");
				exit(-1);
		}
		if (insertObject(item, id, OBJECT_ITEM) == NULL) {
				fprintf(stderr, "Failed to insert object\n");
				return false;
		}
		item->num = num;
		item->object->pos = *pos;
}

static setPos(POSITION* pos, int x, int y) {
		pos->x = x;
		pos->y = y;
}

static void initEvent(eventNotification* event) {
		event->playerId = OWNER;
		event->type = EVENT_NONE;
		event->objId = -1;
		event->id = -1;
		event->pos.x = 0;
		event->pos.y = 0;
		event->angle = 0;
		event->ver = 0;
		event->killTo = -1;
}


void updateBuffer() {
		lastBuffer = &pastAssembly[sub(frame)];
		curBuffer = &pastAssembly[sub(frame + 1)];
		*curBuffer = *lastBuffer;
		lastEvent = &pastEvent[sub(frame)];
		curEvent = &pastEvent[sub(frame + 1)];
		*curEvent = *lastEvent;
		frame++;
		printf("Frame[%d: %d]\n", frame, sub(frame));

		if (averageFromFrequency(FREQ_OBSTACLE)) {
				randomGenerateObstacle();
		}
		if (averageFromFrequency(FREQ_ITEM)) {
				randomGenerateItem();
		}
}


static bool averageFromFrequency(double freq) {
		int accuracy = 4;
		int digit = pow(10, accuracy);
		return rand() % (digit + 1) <= digit * freq / FPS;
}


static bool randomGenerateObstacle() {
		double randAngle = (rand() % (HALF_DEGRESS * 2) - HALF_DEGRESS) * PI / HALF_DEGRESS;
		double randVer = VER_ROCK;
		/* Set random position in map.
		 * Get position on edge of world, that's direction to is last position.
		 * World's shape is circle.
		 */
		double r = MAP_SIZE;
		double a = tan(randAngle);
		int toX, toY;
		do {
				toX = rand() % MAP_SIZE - MAP_SIZE / 2;
				toY = rand() % MAP_SIZE - MAP_SIZE / 2;
		} while (pow(toX, 2) + pow(toY, 2) > pow(MAP_SIZE, 2));
		double b = toY - a * toX;
		double x = (-(a * b) + (cos(randAngle) > 0 ? -1 : 1) * sqrt(pow(a * r, 2) - pow(b, 2) + pow(r, 2))) / (pow(a, 2) + 1);
		double y = (sin(randAngle) > 0 ? -1 : 1) * sqrt(pow(r, 2) - pow(x, 2));
		POSITION randPos = {x, y};
		printf("obstacle pos[%.0f: %.0f]\n", x, y);

		//if (generateObstacle(ownerObject, OBS_ROCK, &randPos, randAngle, randVer)) {
				eventNotification event;
				event.type = EVENT_OBSTACLE;
				event.playerId = OWNER;
				event.objId = OBS_ROCK;
				event.id = ownerObject;
				event.pos = randPos;
				event.angle = randAngle;
				event.ver = randVer;

				ownerObject++;
				return insertEvent(OWNER, &event);
		//} else {
		//		fprintf(stderr, "Failed to random generate obstacle\n");
		//		return false;
		//}
}


static bool randomGenerateItem() {
		ITEM_NUMBER randomNum = rand() % ITEM_NUM;
		POSITION randomPos = {
				rand() % MAP_SIZE - MAP_SIZE / 2,
				rand() % MAP_SIZE - MAP_SIZE / 2
		};

		//if (insertItem(ownerObject, randomNum, &randomPos)) {
				eventNotification event;
				event.type = EVENT_ITEM;
				event.playerId = OWNER;
				event.objId = randomNum;
				event.id = ownerObject;
				event.pos = randomPos;

				ownerObject++;
				return insertEvent(OWNER, &event);
		//} else {
		//		fprintf(stderr, "Failed to random insert item\n");
		//		return false;
		//}
}


static bool insertEvent(int id, eventNotification* event) {
		int i;
		bool endFlag = true;
		printf("event from[%d] type: %d\n", id, event->type);
		for (i = 0; i < clientNum; i++) {
				if (i != id) {
						int j;
						for (j = 0; j < MAX_EVENT; j++) {
								if (curEvent->eventStack[i][j].type == EVENT_NONE) {
										curEvent->eventStack[i][j] = *event;
										printf("player[%d] Success insert event sub[%d]\n", i, j);
										break;
								}
						}
						if (j == MAX_EVENT) {
								printf("player[%d] Event stack is FULL!\n", i);
								endFlag = false;
						}
				}
		}
		return endFlag;
}


static clearEvent(int playerId, int latest) {
		int nowFrame = frame;
		int i;
		for (i = 0; i < MAX_EVENT; i++) {
				eventNotification* eventNtf = &curEvent->eventStack[playerId][i];
				eventNotification* latestNtf = &pastEvent[sub(latest)].eventStack[playerId][i];
				if (eventNtf->type != EVENT_NONE && 
								eventNtf->id == latestNtf->id) {
						initEvent(eventNtf);
				}
		}
#ifndef NDEBUG
		// printf("###clearEvent\n");
#endif
}


void setPlayerState(int id, entityStateSet* state) {
		PLAYER* player = &curBuffer->player[id];
		OBJECT *plyObj = &curBuffer->object[id];
		plyObj->pos.x = state->pos.x;
		plyObj->pos.y = state->pos.y;
#ifndef NDEBUG
		/*
		printf("frame[%d]	player[%d] pos x: %d, y: %d\n", frame, id, plyObj->pos.x, plyObj->pos.y);
		printf("				alive: %d\n", state->player.alive);
		printf("				alive: %d\n", player->alive);
		*/
#endif
		setPlayerValue(player, &state->player);
		
		clearEvent(id, state->latestFrame);
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
		printf("latest[%d: %d]\n", latest, sub(latest));

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
						
						/*
						printf("				alive: %d\n", player->alive);
						*/
				}

				/* イベント所得 */
				for (i = 0; i < MAX_EVENT; i++) {
						eventAssembly *latestEvent = &pastEvent[sub(latest)];
						if (curEvent->eventStack[id][i].type != EVENT_NONE && 
							curEvent->eventStack[id][i].id != latestEvent->eventStack[id][i].id) {
								data.event[i] = curEvent->eventStack[id][i];
						} else {
								data.event[i].type = EVENT_NONE;
						}
				}
				// clearEvent(id, latest);

				printf("send server frame: %d\n", data.latestFrame);
		}

		sendData(id, &data, sizeof(entityStateGet));
}
