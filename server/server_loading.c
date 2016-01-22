#include <stdbool.h>
#include "../common.h"
#include "server_func.h"

static bool state[MAX_CLIENTS];

void initLoading() {
		int i;
		for (i = 0; i < MAX_CLIENTS; i++)
				state[i] = false;

		int clientNum = getClientNum();
}

void nowLoadingClient(int id) {
		state[id] = true;
}
