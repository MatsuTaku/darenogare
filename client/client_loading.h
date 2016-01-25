#ifndef _CLIENT_LOADING_H
#define _CLIENT_LOADING_H

#include <stdbool.h>
#include "../common.h"

#define CHANGE_TIME	3000

extern void initLoading();
extern void finalLoading();
extern bool eventLoading();
extern void updateLoading();
extern void recvLoading(syncData *);
extern void drawLoading(); 

#endif
