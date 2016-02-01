#include "SDL/SDL.h"
#include "SDL/SDL_ttf.h"
#include "common.h"
#include "client_result.h"
#include "client_battle.h"
#include "client_func.h"
#include "client_common.h"

#define CHANGE_TIME	3000;

int winnerId;

static bool enableNextGame;
static int enableTime;

static TTF_Font *swResultFont, *swTitleFont;
static char swResultFontFile[] = "IMG/STJEDISE.TTF";
static SDL_Color orangeColor = {0xEF, 0xB4, 0x02};
static SDL_Color redColor = {0xF2, 0x38, 0x31};
static SDL_Color blueColor = {0x66, 0xff, 0xcc};

void initResult() {
		enableNextGame = false;
		enableTime = SDL_GetTicks() + CHANGE_TIME;

		TTF_Init();
		swResultFont = TTF_OpenFont(swResultFontFile, 90);
		swTitleFont = TTF_OpenFont(swResultFontFile, 48);
}

void finalResult() {
		TTF_CloseFont(swResultFont);
		TTF_CloseFont(swTitleFont);
		TTF_Quit();
}

bool eventResult() {
		SDL_Event event;
		if (SDL_PollEvent(&event)) {
				switch (event.type) {
						case SDL_QUIT:
								return true;
						case SDL_JOYBUTTONDOWN:
								switch (event.jbutton.button) {
										case BUTTON_CIRCLE:
												if (enableNextGame)	changeScene(SCENE_LOADING);
										default:
												break;
								}
						break;
				}
		}
		return false;
}

void updateResult() {
		if (!enableNextGame && SDL_GetTicks() >= enableTime) {
				enableNextGame = !enableNextGame;
		}
}

void drawResult() {
		SDL_Surface *window = SDL_GetVideoSurface();
		SDL_Surface *view = SDL_CreateRGBSurface(0, window->w, window->h, 32, 0, 0, 0, 0);
		SDL_SetAlpha(view, SDL_SRCALPHA, 0x99);

		char resultChar[16];
		SDL_Color *color;
		if (youWin()) {
				sprintf(resultChar, "You win");
				color = &redColor;
		} else {
				sprintf(resultChar, "  You lose...");
				color = &blueColor;
		}
		SDL_Surface *resultString = TTF_RenderUTF8_Blended(swResultFont, resultChar, *color);
		// SDL_SetSurfaceBlendMode(resultString, SDL_BLENDMODE_ADD);
		Rect strRect = {
				.src.w = resultString->w,
				.src.h = resultString->h,
				.dst.x = (window->w - resultString->w) / 2,
				.dst.y = (window->h - resultString->h) / 2 - 120,
		};
		SDL_BlitSurface(resultString, &strRect.src, view, &strRect.dst);
		SDL_FreeSurface(resultString);

		char titleChar[] = "battle finish";
		color = &orangeColor;
		SDL_Surface *titleString = TTF_RenderUTF8_Blended(swTitleFont, titleChar, *color);
		Rect titleRect = {
				.src.w = titleString->w,
				.src.h = titleString->h,
				.dst.x = (window->w - titleString->w) / 2,
				.dst.y = (window->h - titleString->h) / 2 - 190,
		};
		SDL_BlitSurface(titleString, &titleRect.src, view, &titleRect.dst);
		SDL_FreeSurface(titleString);

		if (enableNextGame) {
				char changeChar[] = "press 4 to restart";
				color = &orangeColor;
				SDL_Surface *changeString = TTF_RenderUTF8_Blended(swTitleFont, changeChar, *color);
				Rect changeRect = {
						.src.w = changeString->w,
						.src.h = changeString->h,
						.dst.x = (window->w - changeString->w) / 2,
						.dst.y = (window->h - changeString->h) / 2 + 180,
				};
				SDL_BlitSurface(changeString, &changeRect.src, view, &changeRect.dst);
				SDL_FreeSurface(changeString);
		}

		Rect viewRect = {
				.src.w = view->w,
				.src.h = view->h,
		};
		SDL_BlitSurface(view, &viewRect.src, window, &viewRect.dst);

		SDL_FreeSurface(view);
		SDL_Flip(window);
}
