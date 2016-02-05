#ifndef _CLIENT_SCENE_H_
#define _CLIENT_SCENE_H_

#include <stdbool.h>
#include "../common.h"

extern void sceneFinal();
extern bool sceneManagerEvent();
extern void sceneManagerUpdate();
extern void sceneManagerDraw();
extern bool sceneManagerRecv(syncData *);
extern void sceneManagerSend();
extern void changeScene(SCENE);

#endif
