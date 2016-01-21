#include "client_menu.h"
#include "client_scene.h"
#include "client_common.h"
#include "client_func.h"
#include "SDL/SDL.h"
#include "SDL/SDL_ttf.h"

static char gTitleImgFile[] = "IMG/title.gif";
static char gLoadingImgFile[] = "IMG/tutorial.gif";
static SDL_Surface *gTitleImg;
static SDL_Surface *gLoadingImg;
static char fSWFontFile[] = "IMG/STJEDISE.TTF";
static TTF_Font *fSWFont;

static SDL_Color colorOrange = {0xEF, 0xB4, 0x02};

void initMenu() {
		gTitleImg = loadImage(gTitleImgFile);
		gLoadingImg = loadImage(gLoadingImgFile);

		fSWFont = TTF_OpenFont(fSWFontFile, 60);
}

void finalMenu() {
		SDL_FreeSurface(gTitleImg);
		SDL_FreeSurface(gLoadingImg);
}

void updateMenu() {
		SDL_Surface *window = SDL_GetVideoSurface();
		SDL_FillRect(window, NULL, 0xff000000);
		Rect titleRect = {
				.src.w = gTitleImg->w,
				.src.h = gTitleImg->h,
		};
		SDL_BlitSurface(gTitleImg, &titleRect.src, window, &titleRect.dst);

		char startChar[] = "Press 4 to START";
		SDL_Surface *startString = TTF_RenderUTF8_Blended(fSWFont, startChar, colorOrange);
		Rect startRect = {
				.src.w = startString->w,
				.src.h = startString->h,
				.dst.x = (window->w - startString->w) / 2,
				.dst.h = window->h - 150 - startString->h / 2,
		};
		SDL_BlitSurface(startString, &startRect.src, window, &startRect.dst);

		SDL_Flip(window);
}

void drawMenu() {
}
