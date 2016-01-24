#include "../common.h"
#include "client_common.h"
#include "client_func.h"
#include "client_scene.h"

/** Definitions */
#define nextObj(a)	((a + 1) % MAX_OBJECT)

/** Global Variables */
ASSEMBLY allAssembly;
OBJECT* object;
PLAYER* player;
PLAYER* myPlayer;

/** Static Variables */
static int clientId;
static int clientNum;
static int curObjNum = 0;
static int selfObject;
static eventNotification eventStack[MAX_EVENT];
static int latestFrame = 0;
static entityStateGet getEntity[FRAME_NUM];

/** Static Functions */
static void initObject(OBJECT* object);
static bool deleteObject(int objectId);
static void initPlayer(PLAYER* player, int num, int playerNum);
static bool generateObstacle(int owner, int id,int num, POSITION* pos, double angle, VEROCITY *ver);
static bool insertItem(int id, int num, POSITION* pos);
static OBJECT* insertObject(void* buffer, int id, OBJECT_TYPE type);
static void initEvent();
static void updatePlayer();
static void updateObject();
static void updateObstacle(OBSTACLE* obstacle);
static void collisionDetection();
static void rotateDirection(double sign);
static void accelerateVerocity(double accel);
static void setPlayerPosition();
static void setPos(POSITION* pos, int x, int y);
static bool hitObject(OBJECT* alpha, OBJECT* beta);
static void hitDeleteObject(OBJECT* object);
static void countDownFireLaser();
static void laserPreparation();
static void launchMissile();
static void modeMinimum();
static void updateMinimum();
static void modeBarrier();
static void updateBarrier();
static double getObjectSize(OBJECT* object);
static double getRange(OBJECT* alpha, OBJECT* beta);
static bool judgeSafety(POSITION* pos);
static void launchEvent(eventNotification *event);
static bool insertEvent(eventNotification* event);


/**
 *	ゲームシステムの初期化
 *	return: Error = -1
 */
void initGameSystem(int myId, int playerNum) {
		clientId = myId;
		clientNum = playerNum;
}

int initSystem() {
		int i;
		srand((unsigned)time(NULL));

		for (i = 0; i < FRAME_NUM; i++) {
				getEntity[i].lastFrame = 0;
				getEntity[i].latestFrame = 0;
		}

		object = allAssembly.object;
		player = allAssembly.player;

		for (i = 0; i < MAX_OBJECT; i++) {
				OBJECT* curObj = &object[i];
				initObject(curObj);
		}
		selfObject = 0x1000 * (clientId + 1);

		myPlayer = &player[clientId];

		for (i = 0; i < clientNum; i++) {
				PLAYER* curPlayer = &player[i];
				if (insertObject(curPlayer, i, OBJECT_CHARACTER) == NULL) {
						fprintf(stderr, "Inserting OBJECT is failed!\n");
						return -1;
				}
				initPlayer(curPlayer, i, clientNum);
		}

		return 0;
}


void finalSystem() {
		int i;
		// malloc の消去
		for (i = 0; i < MAX_OBJECT; i++) {
				OBJECT* curObj = &object[i];
				if (curObj->type == OBJECT_OBSTACLE ||
					curObj->type == OBJECT_ITEM) {
						free(curObj->typeBuffer);
				}
		}
}


/** 
 * オブジェクトの挿入
 * input1: オブジェクトバッファ
 */
static void initObject(OBJECT* object) {
		object->type = OBJECT_EMPTY;
		object->id = 0;
		object->typeBuffer = NULL;
		setPos(&object->pos, 0, 0);
}


static bool deleteObject(int objectId) {
		int i;
		OBJECT* curObject;
		for (i = 0; i < MAX_OBJECT; i++) {
				curObject = &object[i];
				if (curObject->type == OBJECT_EMPTY)	continue;
				if (curObject->id == objectId) {
						free(curObject->typeBuffer);
						initObject(curObject);
						printf("delete object[%d]\n", objectId);
						return true;
				}
		}
		printf("failed to delete object[%d]\n", objectId);
		return false;
}


/**
 * プレイヤー初期化
 * input1: プレイヤーポインタ
 * input2: プレイヤーID
 */
static void initPlayer(PLAYER* player, int num, int playerNum) {
		player->num = num;
		player->mode = MODE_NEUTRAL;
		player->modeTime = 0;
		player->item = ITEM_EMPTY;
		player->dir = PI / 2;
		player->toDir = player->dir;
		player->ver.vx = 0;
		player->ver.vy = 0;
		player->alive = true;
		player->boost = BOOST_NEUTRAL;
		player->rotate = ROTATE_NEUTRAL;
		player->action = ACTION_NONE;
		player->bullets = 0;
		player->launchCount = 0;
		player->warn = WARN_SAFETY;
		player->deadTime = 0;
		player->lastTime = 0;
		player->deadAnimation = -1;
		if (playerNum == 1) {
				setPos(&player->object->pos, 0, 0);
		} else {
				double startAngle = START_ANGLE + 2 * PI / playerNum * num;
				POSITION pos = {
						.x = START_RANGE * cos(startAngle),
						.y = -(START_RANGE * sin(startAngle)),
				};
				setPos(&player->object->pos, pos.x, pos.y);
				// printf("player[%d] angle: %f\n", num, startAngle);
		}
}


/**
 * 障害物の挿入
 * return: 成功・失敗
 */
static bool generateObstacle(int owner, int id, int num, POSITION* pos, double angle, VEROCITY *ver) {
		OBSTACLE* curObs;
		if ((curObs = malloc(sizeof(OBSTACLE))) == NULL) {
				fprintf(stderr, "Out of memory! Failed to insert obstacle.\n");
				exit(1);
		}
		if (insertObject(curObs, id, OBJECT_OBSTACLE) == NULL) {
				fprintf(stderr, "Failed to insert object\n");
				return false;
		}
		curObs->object->pos.x = pos->x;
		curObs->object->pos.y = pos->y;
		curObs->owner = owner;
		curObs->num = num;
		curObs->angle = angle;
		curObs->ver = *ver;
}


/**
 * アイテムの挿入
 * input1: アイテム番号
 * input2: ポジション
 * return: 成功・失敗
 */
static bool insertItem(int id, int num, POSITION* pos) {
		ITEM* item;
		if ((item = malloc(sizeof(ITEM))) == NULL) {
				fprintf(stderr, "Out of memory! Failed to insert item.\n");
				exit(1);
		}
		if (insertObject(item, id, OBJECT_ITEM) == NULL) {
				fprintf(stderr, "Failed to insert object\n");
				return false;
		}
		item->num = num;
		item->object->pos = *pos;
		return true;
}


/**
 * オブジェクトの挿入
 * input1: オブジェクト内容バッファ
 * input2: オブジェクトタイプ
 * return: オブジェクトのポインタ(error = NULL)
 */
static OBJECT* insertObject(void* buffer, int id, OBJECT_TYPE type) {
		assert(buffer != NULL);
		assert(type >= 0);
		assert(type < OBJECT_NUM);
		int count = 0;
		OBJECT* curObject;
		while (count < MAX_OBJECT) {
				curObject = &object[curObjNum];
				if (curObject->type == OBJECT_EMPTY) {
						curObject->id = id;
						curObject->type = type;
						curObject->typeBuffer = buffer;
						// 相互参照
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
#ifndef NDEBUG
						// printf("insert object[%d]\n", curObjNum);
#endif
						curObjNum = nextObj(curObjNum);
						return curObject;
				}
				curObjNum = nextObj(curObjNum);
				count++;
		}

		return NULL;
}


/**
 * システムの計算処理
 */
void updateEvent() {
		// initalize event stack
		// initEvent();

		/** Player value change method */
		updatePlayer();
		/** Object value change method */
		updateObject();
		/** launch judging collision detection method */
		collisionDetection();
}


/**
 * イベントスタックの初期化
 */
static void initEvent() {
		int i;
		for (i = 0; i < MAX_EVENT; i++) {
				eventNotification *event = &eventStack[i];
				event->playerId = myPlayer->num;
				event->type = EVENT_NONE;
				event->objId = -1;
				event->id = -1;
				event->killTo = -1;
				setPos(&event->pos, 0, 0);
				event->angle = 0;
				event->ver.vx = 0;
				event->ver.vy = 0;
				event->killTo = 0;
		}
}


/**
 * プレイヤーデータの計算
 */
static void updatePlayer() {
		if (myPlayer->alive) {
				/* プレイヤーの行動 */
				switch (myPlayer->mode) {
						case MODE_NEUTRAL:
								break;
						case MODE_BARRIER:
								updateBarrier();
								break;
						case MODE_MINIMUM:
								updateMinimum();
								break;
						default:
								break;
				}

				/* 行動 */
				switch (myPlayer->action) {
						case ACTION_NONE:	break;
						case ACTION_USE_ITEM:
								switch (myPlayer->item) {
										case ITEM_NOIZING:
												break;
										case ITEM_LASER:
												countDownFireLaser();
												break;
										case ITEM_MISSILE:
												launchMissile();
												break;
										case ITEM_MINIMUM:
												modeMinimum();
												break;
										case ITEM_BARRIER:
												modeBarrier();
												break;
										default:	break;
								}
								break;
						case ACTION_CD_LASER:
								laserPreparation();
								break;
						default:	break;
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
		} else {	// 死亡時処理
#ifndef NDEBUG
				// printf("YOU DIED\n");
#endif
		}
}


static void countDownFireLaser() {
		myPlayer->launchCount = FIRE_TIME_LASER * MIRI_SECOND;
		myPlayer->action = ACTION_CD_LASER;
		myPlayer->item = ITEM_EMPTY;
}

static void laserPreparation() {
		myPlayer->launchCount -= MIRI_SECOND / FPS;
		if (myPlayer->launchCount <= 0) {
				int owner = myPlayer->num;
				int objectId = selfObject++;
				int num = OBS_LASER;
				POSITION *pos = &myPlayer->object->pos;
				double angle = myPlayer->dir;
				VEROCITY ver = {
						.vx = VER_LASER * cos(angle),
						.vy = -VER_LASER * sin(angle),
				};
				generateObstacle(owner, objectId, num, pos, angle, &ver);

				eventNotification event;
				event.playerId = owner;
				event.type = EVENT_OBSTACLE;
				event.id = objectId;
				event.objId = num;
				event.pos = *pos;
				event.angle = angle;
				event.ver = ver;
				insertEvent(&event);

				myPlayer->item = ITEM_EMPTY;
				myPlayer->action = ACTION_NONE;
		}
}


static void launchMissile() {
		myPlayer->bullets--;
		int objectId = selfObject++;
		int num = OBS_MISSILE;
		int owner = myPlayer->num;
		POSITION *pos = &myPlayer->object->pos;
		double angle = myPlayer->dir;
		double absVer = VER_MISSILE;
		VEROCITY ver = {
				.vx = absVer * cos(angle) + myPlayer->ver.vx,
				.vy = absVer * -sin(angle) + myPlayer->ver.vy,
		};
		generateObstacle(owner, objectId, num, pos, angle, &ver);

		eventNotification event = {
				.playerId = myPlayer->num,
				.type = EVENT_OBSTACLE,
				.id = objectId,
				.objId = num,
				.pos = *pos,
				.angle = angle,
				.ver = ver,
		};
		insertEvent(&event);

		if (myPlayer->bullets == 0) {
				myPlayer->item = ITEM_EMPTY;
				myPlayer->action = ACTION_NONE;
		}
}


static void modeMinimum() {
		myPlayer->mode = MODE_MINIMUM;
		myPlayer->item = ITEM_EMPTY;
		myPlayer->modeTime = MODE_TIME_MINIMUM * MIRI_SECOND;
		myPlayer->action = ACTION_NONE;
}

static void updateMinimum() {
		myPlayer->modeTime -= MIRI_SECOND / FPS;
		if (myPlayer->modeTime <= 0) {
				myPlayer->mode = MODE_NEUTRAL;
		}
}

static void modeBarrier() {
		myPlayer->mode = MODE_BARRIER;
		myPlayer->item = ITEM_EMPTY;
		myPlayer->modeTime = MODE_TIME_BARRIER * MIRI_SECOND;
		myPlayer->action = ACTION_NONE;
}

static void updateBarrier() {
		myPlayer->modeTime -= MIRI_SECOND / FPS;
		if (myPlayer->modeTime <= 0) {
				myPlayer->mode = MODE_NEUTRAL;
		}
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
		myPlayer->ver.vy -= accel * sin(direction) / FPS;
		double v = pow(myPlayer->ver.vx, 2) + pow(myPlayer->ver.vy, 2);
		if (v > pow(MAXIMUM_SPEED, 2)) {
				double av = MAXIMUM_SPEED / sqrt(v);
				myPlayer->ver.vx *= av;
				myPlayer->ver.vy *= av;
		}
#ifndef NDEBUG
		// printf("player verocity[|V|: %4.0f, vx: %4.0f, vy: %4.0f]\n", sqrt(v), myPlayer->ver.vx, myPlayer->ver.vy);
#endif
}


/**
 * 機体の向きと速度から座標を移動
 */
static void setPlayerPosition() {
		POSITION* pos = &(myPlayer->object->pos);
		pos->x += myPlayer->ver.vx / FPS;
		pos->y += myPlayer->ver.vy / FPS;
#ifndef NDEBUG
		// printf("player pos[x: %4d, y: %4d]\n", pos->x, pos->y);
#endif
}


/**
 * オブジェクトデータ更新
 */
static void updateObject() {
		int i;
		for (i = 0; i < MAX_OBJECT; i++) {
				OBJECT* curObject = &object[i];
				if (curObject->type != OBJECT_EMPTY) {
						switch (curObject->type) {
								case OBJECT_EMPTY:
										break;
								case OBJECT_CHARACTER:
										break;
								case OBJECT_OBSTACLE:
										updateObstacle((OBSTACLE *)curObject->typeBuffer);
										break;
								case OBJECT_ITEM:
										break;
								default:
										break;
						}
				}
		}

		for (i = 0; i < clientNum; i++) {
				PLAYER* curPlayer = &player[i];
				if (!curPlayer->alive) {
						curPlayer->deadAnimation++;
				}
		}
}


/**
 * 障害物データ更新
 */
static void updateObstacle(OBSTACLE* obstacle) {
		POSITION* pos = &obstacle->object->pos;
		pos->x += obstacle->ver.vx / FPS;
		pos->y += obstacle->ver.vy / FPS;

		// 範囲外の障害物の消去
		if (pow(pos->x, 2) + pow(pos->y, 2) > pow(WORLD_SIZE, 2)) {
				initObject(obstacle->object);
				free(obstacle);
		}
}



/**
 * 当たり判定と処理
 */
static void collisionDetection() {
		if (myPlayer->alive) {
				int i;
				for (i = 0; i < MAX_OBJECT; i++) {
						OBJECT* curObject = &object[i];
						if (curObject->type == OBJECT_EMPTY)	continue;
						switch (curObject->type) {
								case OBJECT_OBSTACLE:
										if (((OBSTACLE *)curObject->typeBuffer)->owner == myPlayer->num)
												break;
										if (hitObject(myPlayer->object, curObject)) {
												if (myPlayer->mode != MODE_BARRIER)
														myPlayer->alive = false;
												hitDeleteObject(curObject);
										}
										break;
								case OBJECT_ITEM:
										if (hitObject(myPlayer->object, curObject)) {
												myPlayer->item = ((ITEM *)curObject->typeBuffer)->num;
												if (myPlayer->item == ITEM_MISSILE)
														myPlayer->bullets = MAX_MISSILE;
												hitDeleteObject(curObject);
										}
										break;
								default:
										break;
						}
				}

				int ms = MIRI_SECOND;
				switch (myPlayer->warn) {
						case WARN_SAFETY:
								if (!judgeSafety(&myPlayer->object->pos)) {
										myPlayer->warn = WARN_OUT_AREA;
										myPlayer->deadTime = SDL_GetTicks() + WARN_TIME_RIMIT * ms;
										myPlayer->lastTime = WARN_TIME_RIMIT * ms;
								}
								break;
						case WARN_OUT_AREA:
								if (judgeSafety(&myPlayer->object->pos)) {
										myPlayer->warn = WARN_SAFETY;
										myPlayer->deadTime = 0;
										myPlayer->lastTime = 0;
								} else {
										if ((myPlayer->lastTime = myPlayer->deadTime - SDL_GetTicks()) < 0) {
												myPlayer->alive = false;
										}
#ifndef NDEBUG
										printf("You are still out of safety-zone!\n");
										printf("come back in [%d] seconds!\n", myPlayer->lastTime / ms);
#endif
								}
								break;
						default:
								break;
				}
		}
}


static void hitDeleteObject(OBJECT* object) {
		eventNotification event;
		event.type = EVENT_DELETE;
		event.playerId = myPlayer->num;
		event.id = object->id;
		insertEvent(&event);

		initObject(object);
}



/**
 * window event emethods
 */

void useItem() {	// アイテムの使用
		if (myPlayer->item != ITEM_EMPTY) {
				myPlayer->action = ACTION_USE_ITEM;
		} else {
				myPlayer->action = ACTION_NONE;
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


// 左旋回
void rotateLeft() {
		myPlayer->rotate = ROTATE_LEFT;
}

// 右旋回
void rotateRight() {
		myPlayer->rotate = ROTATE_RIGHT;
}

// 慣性航行
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
 * オブジェクト座標設定
 *	マップの中央を原点
 */
static void setPos(POSITION* pos, int x, int y) {
		pos->x = x;
		pos->y = y;
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
						switch (((PLAYER *)object->typeBuffer)->mode) {
								case MODE_NEUTRAL:
										size = RANGE_CHARACTER;
										break;
								case MODE_BARRIER:
										size = RANGE_BARRIER;
										break;
								case MODE_MINIMUM:
										size = RANGE_MINIMUM;
										break;
								default:
										size = RANGE_CHARACTER;
										break;
						}
						break;
				case OBJECT_ITEM:
						size = RANGE_ITEM;
						break;
				case OBJECT_OBSTACLE:
						switch (((OBSTACLE *)object->typeBuffer)->num) {
								case OBS_ROCK:
										size = RANGE_ROCK;
										break;
								case OBS_MISSILE:
										size = RANGE_MISSILE;
												break;
								case OBS_LASER:
										size = RANGE_LASER;
												break;
								default:
										break;
						}
						break;
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
		return pow(beta->pos.x - alpha->pos.x, 2) + pow(beta->pos.y - alpha->pos.y, 2);
}


/**
 * 安全圏内にいるかどうか
 * input: ポジション
 * return: 可否
 */
static bool judgeSafety(POSITION* pos) {
		return pow(pos->x, 2) + pow(pos->y, 2) < pow(MAP_SIZE, 2);
}



/***********
 * server method
 */
void reflectDelta(syncData* sData) {
		if (sData->type != DATA_ES_GET)	return;
		entityStateGet *data = &sData->get;
		latestFrame = data->latestFrame;
		bool recieved = data->lastFrame > getEntity[FRAME_LAST].lastFrame;
		getEntity[FRAME_LAST] = getEntity[FRAME_LATEST];
		getEntity[FRAME_LATEST] = *data;

		DELTA* delta = &data->delta;
		int i;
		for (i = 0; i < MAX_CLIENTS; i++) {
				if (i != myPlayer->num && player[i].object != NULL) {
						PLAYER* curPlayer = &player[i];
						PLAYER* deltaPlayer = &delta->player[i];
						OBJECT* curObject = curPlayer->object;
						OBJECT* deltaObject = &delta->plyObj[i];
						if (recieved) {
								curPlayer->mode += deltaPlayer->mode;
								curPlayer->modeTime += deltaPlayer->modeTime;
								curPlayer->dir += deltaPlayer->dir;
								curPlayer->toDir += deltaPlayer->toDir;
								curPlayer->ver.vx += deltaPlayer->ver.vx;
								curPlayer->ver.vy += deltaPlayer->ver.vy;
								curPlayer->alive = deltaPlayer->alive;
								curPlayer->boost += deltaPlayer->boost;
								curPlayer->rotate += deltaPlayer->rotate;
								curPlayer->action += deltaPlayer->action;
								curPlayer->item += deltaPlayer->item;
								curPlayer->bullets += deltaPlayer->bullets;
								curPlayer->launchCount += deltaPlayer->launchCount;
								curPlayer->warn += deltaPlayer->warn;
								curPlayer->deadTime +=deltaPlayer->deadTime;
								curPlayer->lastTime += deltaPlayer->lastTime;

								curObject->pos.x += deltaObject->pos.x;
								curObject->pos.y += deltaObject->pos.y;
						} else {
#ifndef NDEBUG
								printf("!!!!!fallthrough!!!!!\n");
#endif
								DELTA* lastDelta = &getEntity[FRAME_LAST].delta;
								PLAYER* lastPlayer = &lastDelta->player[i];
								OBJECT* lastObject = &lastDelta->plyObj[i];
								curPlayer->mode += deltaPlayer->mode - lastPlayer->mode;
								curPlayer->modeTime += deltaPlayer->modeTime - lastPlayer->modeTime;
								curPlayer->dir += deltaPlayer->dir - lastPlayer->dir;
								curPlayer->toDir += deltaPlayer->toDir - lastPlayer->toDir;
								curPlayer->ver.vx += deltaPlayer->ver.vx - lastPlayer->ver.vx;
								curPlayer->ver.vy += deltaPlayer->ver.vy - lastPlayer->ver.vy;
								curPlayer->alive = deltaPlayer->alive;
								curPlayer->boost += deltaPlayer->boost - lastPlayer->boost;
								curPlayer->rotate += deltaPlayer->rotate - lastPlayer->rotate;
								curPlayer->action += deltaPlayer->action - lastPlayer->action;
								curPlayer->item += deltaPlayer->item - lastPlayer->item;
								curPlayer->bullets += deltaPlayer->bullets - lastPlayer->bullets;
								curPlayer->launchCount += deltaPlayer->launchCount - lastPlayer->launchCount;
								curPlayer->warn += deltaPlayer->warn - lastPlayer->warn;
								curPlayer->deadTime +=deltaPlayer->deadTime - lastPlayer->deadTime;
								curPlayer->lastTime += deltaPlayer->lastTime - lastPlayer->lastTime;

								curObject->pos.x += deltaObject->pos.x - lastObject->pos.x;
								curObject->pos.y += deltaObject->pos.y - lastObject->pos.y;
						}
#ifndef NDEBUG
						// printf("player[%d] pos x: %d, y: %d\n", i, curObject->pos.x, curObject->pos.y);
						// printf("		alive: %d\n", curPlayer->alive);
#endif
				}
		}

		for (i = 0; i < MAX_EVENT; i++) {
				eventNotification *curEvent = &data->event[i];
				if (curEvent->type != EVENT_NONE)
						launchEvent(curEvent);
		}

#ifndef NDEBUG
		// printf("Frame[%d : %d]\n", data->latestFrame, data->lastFrame);
#endif
}


static void launchEvent(eventNotification *event) {
#ifndef NDEBUG
		// printf("launchEvent type: %d\n", event->type);
#endif
		switch (event->type) {
				case EVENT_OBSTACLE:
						generateObstacle(event->playerId, event->id, event->objId, &event->pos, event->angle, &event->ver);
						break;
				case EVENT_DELETE:
						deleteObject(event->id);
						break;
				case EVENT_ITEM:
						insertItem(event->id, event->objId, &event->pos);
						break;
				case EVENT_KILL:
						break;
				default:
						break;
		}
}


static bool insertEvent(eventNotification* event) {
		int i;
		for (i = 0; i < MAX_EVENT; i++) {
				if (eventStack[i].type == EVENT_NONE) {
						eventStack[i] = *event;
						printf("Success insert event sub[%d]\n", i);
						return true;
				}
		}
		printf("Event stack is FULL!\n");
		return false;
}


void sendEntity() {
		syncData data = {
				.set = {
						.type = DATA_ES_SET,
						.latestFrame = latestFrame,
						.endFlag = false,
						.clientId = clientId,
						.pos = myPlayer->object->pos,
						.player = *myPlayer,
				},
		};
		int i;
		for (i = 0; i < MAX_EVENT; i++)	data.set.event[i] = eventStack[i];
		initEvent();

		sendData(&data, sizeof(syncData));
#ifndef NDEBUG
		printf("sendEntity frame: %d	time: %d\n", latestFrame, SDL_GetTicks());
#endif
}
