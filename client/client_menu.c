#include "client_menu.h"
#include "client_scene.h"
#include "client_func.h"
#include "SDL/SDL.h"

static char gStartImgFile[] = "";
static char gLoadingImgFile[] = "";
static SDL_Surface *gStartImg;
static SDL_Surface *gLoadingImg;

static int menuIndex = 0;

void initMenu() {
		gStartImg = loadImage(gStartImgFile);
		gLoadingImg = loadImage(gLoadingImgFile);
}

void finalMenu() {
		SDL_FreeSurface(gStartImg);
		SDL_FreeSurface(gLoadingImg);
}

void updateMenu() {
}

void drawMenu() {
}
