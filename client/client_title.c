#include "client_title.h"
#include "client_scene.h"
#include "client_common.h"
#include "client_func.h"
#include "SDL/SDL.h"
#include "SDL/SDL_ttf.h"

#define BG_NUM	5

static char gBGImgFile[][32] = {
		"IMG/stars.gif",
		"IMG/stars2.gif",
		"IMG/stars3.gif",
		"IMG/stars4.gif",
		"IMG/stars5.gif",
};
static char gTitleImgFile[] = "IMG/title.gif";
static SDL_Surface *gBGImg;
static SDL_Surface *gTitleImg;
static SDL_Surface *gLoadingImg;
static char fSWFontFile[] = "IMG/STJEDISE.TTF";
static TTF_Font *fSWFont;
static SDL_Color colorOrange = {0xEF, 0xB4, 0x02};

static bool loadingFlag;

void initTitle() {
		printf("initTitle\n");

		int i;
		srand((unsigned)time(NULL));

		loadingFlag = false;

		gTitleImg = loadImage(gTitleImgFile);
		int BGIndex = rand() % BG_NUM;
		gBGImg = loadImage(gBGImgFile[BGIndex]);

		TTF_Init();
		fSWFont = TTF_OpenFont(fSWFontFile, 60);
}

void finalTitle() {
		printf("finalTitle\n");

		int i;
		SDL_FreeSurface(gTitleImg);
		SDL_FreeSurface(gBGImg);

		TTF_CloseFont(fSWFont);
		TTF_Quit();
}

bool eventTitle() {
		SDL_Event event;
		while (SDL_PollEvent(&event)) {
				switch (event.type) {
						case SDL_QUIT:
								return true;			
						case SDL_JOYBUTTONDOWN:
								switch (event.jbutton.button) {
										case BUTTON_CIRCLE:
												loadingFlag = true;
												break;
								}
								break;
						default:
								break;
				}
		}
		return false;
}

void updateTitle() {
		if (loadingFlag) {
				changeScene(SCENE_LOADING);
		}
}

void drawTitle() {
		SDL_Surface *window = SDL_GetVideoSurface();
		SDL_FillRect(window, NULL, 0xff000000);

		Rect titleRect = {
				.src.w = gTitleImg->w,
				.src.h = gTitleImg->h,
				.dst.x = (window->w - gTitleImg->w) / 2,
				.dst.y = 200 - gTitleImg->h / 2,
		};
		SDL_BlitSurface(gTitleImg, &titleRect.src, window, &titleRect.dst);

		char startChar[] = "Press 4 to START";
		SDL_Surface *startString = TTF_RenderUTF8_Blended(fSWFont, startChar, colorOrange);
		Rect startRect = {
				.src.w = startString->w,
				.src.h = startString->h,
				.dst.x = (window->w - startString->w) / 2,
				.dst.y = window->h - 150 - startString->h / 2,
		};
		SDL_BlitSurface(startString, &startRect.src, window, &startRect.dst);
}
