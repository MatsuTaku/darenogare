#include <stdbool.h>
#include "../common.h"
#include "server_func.h"

static bool state[MAX_CLIENTS];
static int clientNum;

static void callStart();

void initLoading() {
		int i;
		for (i = 0; i < MAX_CLIENTS; i++)
				state[i] = false;

		clientNum = getClientNum();
}

void nowLoadingClient(int id) {
		if (!state[id])
				printf("client[%d] is OK\n", id);
		state[id] = true;
}

void updateLoading() {
		int i;
		bool startFlag = true;
		for (i = 0; i < clientNum; i++) {
				if (!state[i]) {
						startFlag = false;
				}
		}
		if (startFlag) {
				callStart();
				changeScene(SCENE_BATTLE);
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
