#include "client_loading.h"
#include "client_scene.h"
#include "client_common.h"
#include "client_func.h"
#include "SDL/SDL.h"

static char gLoadingImgFile[] = "IMG/tutorial.gif";
static SDL_Surface *gLoadingImg;

void initLoading() {
		printf("initLoading\n");

		gLoadingImg = loadImage(gLoadingImgFile);
}

void finalLoading() {
		printf("finalLoading\n");

		SDL_FreeSurface(gLoadingImg);
}

bool eventLoading() {
		SDL_Event event;
		while (SDL_PollEvent(&event)) {
				switch (event.type) {
						case SDL_QUIT:
								return true;
						case SDL_JOYBUTTONDOWN:
								break;
						default:
								break;
				}
		}
		return false;
}

void updateLoading() {
}

void drawLoading() {
		SDL_Surface *window = SDL_GetVideoSurface();

		SDL_FillRect(window, NULL, 0xff000000);

		if (gLoadingImg) {
				Rect rect = {
						.src.w = gLoadingImg->w,
						.src.h = gLoadingImg->h,
				};
				SDL_BlitSurface(gLoadingImg, &rect.src, window, &rect.dst);
		} else {
				printf("loadingimg is NULL\n");
		}

		SDL_Flip(window);
}
