#include "client_scene.h"
#include "client_func.h"
#include "client_menu.h"

static SCENE scene = SCENE_PREPARE;
static SCENE nextScene = SCENE_NONE;

static void sceneInitModule(SCENE cScene);
static void sceneFinalModule(SCENE cScene);

void sceneInit() {
		sceneInitModule(scene);
}


void sceneFinal() {
		sceneFinalModule(scene);
}


void sceneManagerUpdate() {
		switch (scene) {
				case SCENE_PREPARE:
						updateMenu();
						break;
				case SCENE_BATTLE:
						updateEvent();
						break;
				default:
						break;
		}
		if (nextScene != SCENE_NONE) {
				sceneFinalModule(scene);
				scene = nextScene;
				nextScene = SCENE_NONE;
				sceneInitModule(scene);
		}
}


void sceneManagerDraw() {
		switch (scene) {
				case SCENE_PREPARE:
						drawMenu();
						break;
				case SCENE_BATTLE:
						drawWindow();
						break;
				default:
						break;
		}
}


void changeScene(SCENE newScene) {
		scene = newScene;
}


/********* static **********/

static void sceneInitModule(SCENE cScene) {
		switch (cScene) {
				case SCENE_PREPARE:
						initMenu();
						break;
				case SCENE_BATTLE:
						initSystem();
						break;
				default:
						break;
		}
}


static void sceneFinalModule(SCENE cScene) {
		switch (cScene) {
				case SCENE_PREPARE:
						finalMenu();
						break;
				case SCENE_BATTLE:
						finalSystem();
						break;
				default:
						break;
		}
}
