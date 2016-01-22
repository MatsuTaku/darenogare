#include "server_scene.h"
#include "server_func.h"
#include "server_loading.h"

static SCENE mScene = SCENE_TITLE;
static SCENE mNextScene = SCENE_NONE;

static void sceneInitModule(SCENE scene);

void sceneInit() {
		sceneInitModule(mScene);
}

bool sceneManagerSync() {
		bool endFlag;
		if (mNextScene != SCENE_NONE) {
				mScene = mNextScene;
				mNextScene = SCENE_NONE;
				sceneInit(mScene);
		}
		switch (mScene) {
				case SCENE_LOADING:
						endFlag = loadingSync();
						break;
				case SCENE_BATTLE:
						endFlag = battleSync();
						break;
				default:
						endFlag = false;
						break;
		}
		return endFlag;
}

void changeScene(SCENE scene) {
		mNextScene = scene;
}


/********** static ******/
static void sceneInitModule(SCENE scene) {
		switch (scene) {
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
