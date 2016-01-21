#ifndef _CLIENT_SCENE_H_
#define _CLIENT_SCENE_H_

#include <stdbool.h>

typedef enum {
		SCENE_NONE,
		SCENE_TITLE,
		SCENE_LOADING,
		SCENE_BATTLE,
} SCENE;

extern void sceneInit();
extern void sceneFinal();
extern bool sceneManagerEvent();
extern void sceneManagerUpdate();
extern void sceneManagerDraw();
extern void changeScene(SCENE);

#endif
