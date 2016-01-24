#include "client_scene.h"
#include "client_func.h"
#include "client_title.h"

static SCENE scene = SCENE_TITLE;
static SCENE nextScene = SCENE_NONE;

static void switchScene();
static void sceneInitModule(SCENE cScene);
static void sceneFinalModule(SCENE cScene);

void sceneInit() {
		sceneInitModule(scene);
}


void sceneFinal() {
		sceneFinalModule(scene);
}


bool sceneManagerEvent() {
		switchScene();
		switch (scene) {
				case SCENE_TITLE:
						return eventTitle();
				case SCENE_LOADING:
						return eventLoading();
				case SCENE_BATTLE:
						return windowEvent();
				default:
						break;
		}
		return false;
}

void sceneManagerUpdate() {
		switch (scene) {
				case SCENE_TITLE:
						updateTitle();
						break;
				case SCENE_LOADING:
						updateLoading();
						break;
				case SCENE_BATTLE:
						updateEvent();
						break;
				default:
						break;
		}
}


void sceneManagerDraw() {
		switch (scene) {
				case SCENE_TITLE:
						drawTitle();
						break;
				case SCENE_LOADING:
						drawLoading();
						break;
				case SCENE_BATTLE:
						drawWindow();
						break;
				default:
						break;
		}
}


bool sceneManagerRecv(syncData *data) {
		if (data->common.endFlag)
				return true;
		switch (scene) {
				case SCENE_TITLE:
						break;
				case SCENE_LOADING:
						recvLoading(data);
						break;
				case SCENE_BATTLE:
						reflectDelta(data);
						break;
				default:
						break;
		}
		return false;
}


void sceneManagerSend() {
		switch (scene) {
				case SCENE_TITLE:
						break;
				case SCENE_LOADING:
						break;
				case SCENE_BATTLE:
						sendEntity();
						break;
				default:
						break;
		}
}


void changeScene(SCENE newScene) {
		nextScene = newScene;
}


/********* static **********/

static void switchScene() {
		if (nextScene != SCENE_NONE) {
				sceneFinalModule(scene);
				scene = nextScene;
				nextScene = SCENE_NONE;
				sceneInitModule(scene);
#ifndef NDEBUG
				printf("Change scene: %d\n", scene);
#endif
		}
}

static void sceneInitModule(SCENE cScene) {
		switch (cScene) {
				case SCENE_TITLE:
						initTitle();
						break;
				case SCENE_LOADING:
						initLoading();
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
				case SCENE_TITLE:
						finalTitle();
						break;
				case SCENE_LOADING:
						finalLoading();
						break;
				case SCENE_BATTLE:
						finalSystem();
						break;
				default:
						break;
		}
}
