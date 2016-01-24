#ifndef _CLIENT_LOADING_H
#define _CLIENT_LOADING_H

#include <stdbool.h>
#include "../common.h"

extern void initLoading();
extern void finalLoading();
extern bool eventLoading();
extern void updateLoading();
extern void recvLoading(syncData *);
extern void drawLoading(); 

#endif
