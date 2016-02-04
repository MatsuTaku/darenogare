#include "client_loading.h"
#include "client_scene.h"
#include "client_common.h"
#include "client_func.h"
#include "common.h"
#include "SDL/SDL.h"
#include "SDL/SDL_ttf.h"

static char gLoadingImgFile[] = "IMG/tutorial.gif";
static SDL_Surface *gLoadingImg;
static int mChangeTime;
static bool mChangeFlag;
static int mViewAlpha;
static char fSWFontFile[] = "IMG/STJEDISE.TTF";
static TTF_Font *fSWFont;
static SDL_Color colorOrange = {0xEF, 0xB4, 0x02};

void initLoading() {
		printf("initLoading\n");

		mChangeFlag = false;

		gLoadingImg = loadImage(gLoadingImgFile);

		TTF_Init();
		fSWFont = TTF_OpenFont(fSWFontFile, 20);
}

void finalLoading() {
		printf("finalLoading\n");

		SDL_FreeSurface(gLoadingImg);
		SDL_FreeSurface(NULL);

		TTF_CloseFont(fSWFont);
		TTF_Quit();
}

bool eventLoading() {
		SDL_Event event;
		while (SDL_PollEvent(&event)) {
				switch (event.type) {
						case SDL_QUIT:
								return true;
						default:
								break;
				}
		}
		return false;
}

void updateLoading() {
		if (!mChangeFlag) {
				syncData data = {
						.prepare.type = DATA_PREPARE,
						.prepare.endFlag = false,
				};
				sendData(&data, sizeof(syncData));
		} else  if (mChangeTime < SDL_GetTicks()) {
				changeScene(SCENE_BATTLE);
		}
}

void recvLoading(syncData *data) {
		printf("#recvLoading[%d]\n", data->type);
		if (data->type == DATA_PREPARE) {
				mChangeFlag = true;
				mChangeTime = SDL_GetTicks() + CHANGE_TIME;
		}
}

void drawLoading() {
		SDL_Surface *window = SDL_GetVideoSurface();

		SDL_FillRect(window, NULL, 0xff000000);

		Rect rect = {
				.src.w = gLoadingImg->w,
				.src.h = gLoadingImg->h,
				.dst.x = (window->w - gLoadingImg->w) / 2,
				.dst.y = (window->h - gLoadingImg->h) / 2,
		};
		int alpha = 0xff;
		int lastTime = mChangeTime - SDL_GetTicks();
		if (mChangeFlag && lastTime < CHANGE_TIME / 3) {
				alpha = 0xff * lastTime / (CHANGE_TIME / 3);
		}
		SDL_SetAlpha(gLoadingImg, SDL_SRCALPHA, alpha);
		SDL_BlitSurface(gLoadingImg, &rect.src, window, &rect.dst);

		if (mChangeFlag) {
				char lastTimeChar[4];
				sprintf(lastTimeChar, "%d", lastTime / 1000 + 1);
				SDL_Surface *lastTime = TTF_RenderUTF8_Blended(fSWFont, lastTimeChar, colorOrange);
				Rect strRect = {
						.src.w = lastTime->w,
						.src.h = lastTime->h,
						.dst.x = window->w / 2 - lastTime->w + gLoadingImg->w / 2 - 180,
						.dst.y = window->h / 2 - lastTime->h + gLoadingImg->h / 2 - 2,
				};
				SDL_BlitSurface(lastTime, &strRect.src, window, &strRect.dst);
		}
}
