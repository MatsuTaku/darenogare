#ifndef _CLIENT_SCENE_H_
#define _CLIENT_SCENE_H_

typedef enum {
		SCENE_NONE,
		SCENE_MENU,
		SCENE_BATTLE,
} SCENE;

extern void sceneInit();
extern void sceneFinal();
extern void sceneManagerUpdate();
extern void sceneManagerDraw();
extern void changeScene(SCENE);

#endif
