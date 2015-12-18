#include "../common.h"
#include "client_common.h"
#include "client_func.h"

#define nextObj(a)	((a + 1) % MAX_OBJECT)

ASSEMBLY allAssembly;
OBJECT* object;
PLAYER* player;
PLAYER* myPlayer;
static int curObjNum;

static int latestFrame;
static entityStateGet getEntity[FRAME_NUM];

static void initObject(OBJECT* object);
static void initPlayer(PLAYER* player, int num);
static void initObstacle(OBSTACLE* obstacle);
static bool generateObstacle();
static bool insertItem(int num, POSITION* pos);
static OBJECT* insertObject(void* buffer, OBJECT_TYPE type);
static void updatePlayer();
static void updateObject();
static void updateObstacle(OBSTACLE* obstacle);
static void collisionDetection();
static void rotateDirection(double sign);
static void accelerateVerocity(double accel);
static void setPlayerPosition();
static void setPos(OBJECT* object, int x, int y);
static bool hitObject(OBJECT* alpha, OBJECT* beta);
static double getObjectSize(OBJECT* object);
static double getRange(OBJECT* alpha, OBJECT* beta);
static bool judgeSafety(POSITION* pos);


/**
 *	ゲームシステムの初期化
 *	return: Error = -1
 */
int initGameSystem(int myId, int playerNum) {
		int i;
		srand((unsigned)time(NULL));

		latestFrame = 0;
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
		curObjNum = 0;

		myPlayer = &player[myId];

		for (i = 0; i < playerNum; i++) {
				PLAYER* curPlayer = &player[i];
				if (insertObject(curPlayer, OBJECT_CHARACTER) == NULL) {
						fprintf(stderr, "Inserting OBJECT is failed!\n");
						return -1;
				}
				initPlayer(curPlayer, i);
		}

		// test appearance
		for (i = 0; i < MAX_OBSTACLE; i++) {
				if (!generateObstacle()) {
						fprintf(stderr, "Failed to insert obstacle\n");
						return -1;
				}
		}

		for (i = 0; i < MAX_ITEM; i++) {
				int num = rand() % ITEM_NUM;
				POSITION pos = {
						rand() % MAP_SIZE - MAP_SIZE / 2,
						rand() % MAP_SIZE - MAP_SIZE / 2
				};
				if (!insertItem(num, &pos)) {
						fprintf(stderr, "Failed to insert item\n");
						return -1;
				}
		}

		return 0;
}


/** 
 * オブジェクトの挿入
 * input1: オブジェクトバッファ
 */
static void initObject(OBJECT* object) {
		object->type = OBJECT_EMPTY;
		object->id = 0;
		setPos(object, 0, 0);
}


/**
 * プレイヤー初期化
 * input1: プレイヤーポインタ
 * input2: プレイヤーID
 */
static void initPlayer(PLAYER* player, int num) {
		player->num = num;
		player->item = ITEM_EMPTY;
		player->dir = PI / 2;
		player->toDir = player->dir;
		player->ver.vx = 0;
		player->ver.vy = 0;
		player->alive = true;
		player->boost = BOOST_NEUTRAL;
		player->rotate = ROTATE_NEUTRAL;
		player->action = ACTION_NONE;
		player->warn = WARN_SAFETY;
		player->deadTime = 0;
		player->lastTime = 0;
		setPos(player->object, 0, 0);
}


static void initObstacle(OBSTACLE* obstacle) {
		double angle = (rand() % (HALF_DEGRESS * 2) - HALF_DEGRESS + 1) * PI / HALF_DEGRESS;
		obstacle->angle = angle;
		obstacle->ver = MAXIMUM_SPEED_OBSTACLE;

		/* マップ内に目標点をランダムに設定。
		 * その座標に向かうように初期点をワールドの端に設定する。
		 * ワールドは円形。
		 */
		// double r = WORLD_SIZE;
		double r = MAP_SIZE;
		double a = sin(angle) / cos(angle);
		int toX, toY;
		do {
				toX = rand() % MAP_SIZE - MAP_SIZE / 2;
				toY = rand() % MAP_SIZE - MAP_SIZE / 2;
		} while (pow(toX, 2) + pow(toY, 2) > pow(MAP_SIZE, 2));
		double b = toY - a * toX;
		double x, y;
		x =	(-(a * b) + (cos(angle) > 0 ? -1 : 1) * sqrt(pow(a * r, 2) - pow(b, 2) + pow(r, 2)))
				/ (pow(a, 2) + 1);
		y = (sin(angle) > 0 ? -1 : 1) * sqrt(pow(r, 2) - pow(x, 2)); 
		setPos(obstacle->object, x, y);
}


/**
 * 障害物の挿入
 * return: 成功・失敗
 */
static bool generateObstacle() {
		OBSTACLE* curObs;
		if ((curObs = malloc(sizeof(OBSTACLE))) == NULL) {
				fprintf(stderr, "Out of memory! Failed to insert obstacle.\n");
				exit(1);
		}
		if (insertObject(curObs, OBJECT_OBSTACLE) == NULL) {
				fprintf(stderr, "Failed to insert object\n");
				return false;
		}
		initObstacle(curObs);
		return true;
}


/**
 * アイテムの挿入
 * input1: アイテム番号
 * input2: ポジション
 * return: 成功・失敗
 */
static bool insertItem(int num, POSITION* pos) {
		ITEM* item;
		if ((item = malloc(sizeof(ITEM))) == NULL) {
				fprintf(stderr, "Out of memory! Failed to insert item.\n");
				exit(1);
		}
		if (insertObject(item, OBJECT_ITEM) == NULL) {
				fprintf(stderr, "Failed to insert object\n");
				return false;
		}
		item->num = num;
		setPos(item->object, pos->x, pos->y);
		return true;
}


/**
 * オブジェクトの挿入
 * input1: オブジェクト内容バッファ
 * input2: オブジェクトタイプ
 * return: オブジェクトのポインタ(error = NULL)
 */
static OBJECT* insertObject(void* buffer, OBJECT_TYPE type) {
		assert(buffer != NULL);
		assert(type >= 0);
		assert(type < OBJECT_NUM);
		int count = 0;
		OBJECT* curObject;
		while (count < MAX_OBJECT) {
				curObject = &object[curObjNum];
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


/**
 * システムの計算処理
 */
void updateEvent() {
		/** Player value change method */
		updatePlayer();

		updateObject();

		collisionDetection();
}


/**
 * プレイヤーデータの計算
 */
static void updatePlayer() {
		if (myPlayer->alive) {
				/* プレイヤーの行動 */
				// MARK
				switch (myPlayer->action) {
						case ACTION_NONE:	break;
						case ACTION_USE_ITEM:
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
		} else {	// 死亡時処理
#ifndef NDEBUG
				printf("You dead\n");
#endif
		}
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
}


/**
 * 障害物データ更新
 */
static void updateObstacle(OBSTACLE* obstacle) {
		double ver = obstacle->ver;
		double angle = obstacle->angle;
		POSITION* pos = &obstacle->object->pos;
		pos->x += ver * cos(angle) / FPS;
		pos->y += ver * sin(angle) / FPS;
}


/**
 * 当たり判定と処理
 */
static void collisionDetection() {
		int i;
		for (i = 0; i < MAX_OBJECT; i++) {
				OBJECT* curObject = &object[i];
				if (curObject->type > OBJECT_EMPTY) {
						switch (curObject->type) {
								case OBJECT_OBSTACLE:
										if (hitObject(myPlayer->object, curObject)) {
												myPlayer->alive = false;
												initObject(curObject);
										}
								case OBJECT_ITEM:
										if (hitObject(myPlayer->object, curObject)) {
												myPlayer->item = ((ITEM *)curObject->typeBuffer)->num;
												initObject(curObject);
										}
								default:
										break;
						}
				}
		}

		int ms = 1000;
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
#ifndef NDEBUG
										printf("You are still out of safety-zone!\n");
										printf("come back in [%d] seconds!\n", myPlayer->lastTime);
#endif
								}
						}
						break;
				default:
						break;
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
								myPlayer->item = ((ITEM*)curObj->typeBuffer)->num;
								initObject(curObj);
								break;
						}
				}
		}
}


void useItem() {	// アイテムの使用
		if (myPlayer->item != ITEM_EMPTY) {
				myPlayer->action = ACTION_USE_ITEM;
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
static void setPos(OBJECT* object, int x, int y) {
		object->pos.x = x;
		object->pos.y = y;
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
void reflectDelta(entityStateGet* data) {
		latestFrame = data->latestFrame;
		bool recieved = false;
		if (data->lastFrame > getEntity[FRAME_LAST].lastFrame)
				recieved = true;
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
								curPlayer->dir += deltaPlayer->dir;
								curPlayer->toDir += deltaPlayer->toDir;
								curPlayer->ver.vx += deltaPlayer->ver.vx;
								curPlayer->ver.vy += deltaPlayer->ver.vy;
								if (deltaPlayer->alive)
										curPlayer->alive = !curPlayer->alive;
								curPlayer->boost += deltaPlayer->boost;
								curPlayer->rotate += deltaPlayer->rotate;
								curPlayer->action += deltaPlayer->action;
								curPlayer->item += deltaPlayer->item;
								curPlayer->warn += deltaPlayer->warn;
								curPlayer->deadTime +=deltaPlayer->deadTime;
								curPlayer->lastTime += deltaPlayer->lastTime;

								curObject->pos.x += deltaObject->pos.x;
								curObject->pos.y += deltaObject->pos.y;
						} else {
								printf("fallthrough\n");
								DELTA* lastDelta = &getEntity[FRAME_LAST].delta;
								PLAYER* lastPlayer = &lastDelta->player[i];
								OBJECT* lastObject = &lastDelta->plyObj[i];
								curPlayer->dir += deltaPlayer->dir - lastPlayer->dir;
								curPlayer->toDir += deltaPlayer->toDir - lastPlayer->toDir;
								curPlayer->ver.vx += deltaPlayer->ver.vx - lastPlayer->ver.vx;
								curPlayer->ver.vy += deltaPlayer->ver.vy - lastPlayer->ver.vy;
								if (deltaPlayer->alive != lastPlayer->alive)
										curPlayer->alive = !curPlayer->alive;
								curPlayer->boost += deltaPlayer->boost - lastPlayer->boost;
								curPlayer->rotate += deltaPlayer->rotate - lastPlayer->rotate;
								curPlayer->action += deltaPlayer->action - lastPlayer->action;
								curPlayer->item += deltaPlayer->item - lastPlayer->item;
								curPlayer->warn += deltaPlayer->warn - lastPlayer->warn;
								curPlayer->deadTime +=deltaPlayer->deadTime - lastPlayer->deadTime;
								curPlayer->lastTime += deltaPlayer->lastTime - lastPlayer->lastTime;

								curObject->pos.x += deltaObject->pos.x - lastObject->pos.x;
								curObject->pos.y += deltaObject->pos.y - lastObject->pos.y;
						}
#ifndef NDEBUG
						printf("player[%d] pos x: %d, y: %d\n", i, curObject->pos.x, curObject->pos.y);
						printf("		alive: %d\n", curPlayer->alive);
#endif
				}
		}
#ifndef NDEBUG
		printf("Frame[%d : %d]\n", data->latestFrame, data->lastFrame);
		printf("recieve time: %d\n", SDL_GetTicks());
#endif
}


void sendEntity() {
		entityStateSet data;
		data.latestFrame = latestFrame;
		data.endFlag = false;
		data.clientId = myPlayer->num;
		data.pos.x = myPlayer->object->pos.x;
		data.pos.y = myPlayer->object->pos.y;
		data.player = *myPlayer;
		data.killEnemy = -1;

		sendData(&data, sizeof(entityStateSet));
#ifndef NDEBUG
		printf("sendEntity frame: %d	time: %d\n", latestFrame, SDL_GetTicks());
#endif
}
