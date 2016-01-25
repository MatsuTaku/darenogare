#include <stdbool.h>
#include "server_loading.h"
#include "../common.h"
#include "server_func.h"

static bool state[MAX_CLIENTS];
static int clientNum;
static int changeTime;
static bool changeFlag;

static void callStart();

void initLoading() {
		int i;
		for (i = 0; i < MAX_CLIENTS; i++)
				state[i] = false;

		clientNum = getClientNum();
		changeFlag = false;
}

void nowLoadingClient(int id) {
		if (!state[id])
				printf("client[%d] is OK\n", id);
		state[id] = true;
}

void updateLoading() {
		if (!changeFlag) {
				int i;
				bool startFlag = true;
				for (i = 0; i < clientNum; i++) {
						if (!state[i]) {
								startFlag = false;
						}
				}
				if (startFlag) {
						changeFlag = startFlag;
						changeTime = SDL_GetTicks() + LOAD_TIME_MIN;
						callStart();
				}
		} else {
				if (SDL_GetTicks() >= changeTime) {
						changeScene(SCENE_BATTLE);
				}
		}
}

/************ static ***********/
static void callStart() {
		syncData data = {
				.prepare.type = DATA_PREPARE,
				.prepare.endFlag = false,
		};
		sendData(ALL_CLIENTS, &data, sizeof(syncData));

		printf("BATTLE Start!!!\n");
}
