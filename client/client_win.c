#include <SDL/SDL.h>
#include <SDL/SDL_image.h>
#include <SDL/SDL_gfxPrimitives.h>
#include <SDL/SDL_ttf.h>
#include "../common.h"
#include "client_common.h"
#include "client_func.h"

#define VIEW_WIDTH	1280
#define VIEW_HEIGHT	720

static int weitFlag = 0;
static int myID;

static SDL_Surface *gMainWindow;

int initWindows(int clientID, int num)
{
		int i;
		char *s, title[10];
		myID = clientID;

		assert(0<num && num<=MAX_CLIENTS);

		if(SDL_Init(SDL_INIT_VIDEO) < 0) {
				printf("failed to initialize SDL.\n");
				return -1;
		}

		if((gMainWindow = SDL_SetVideoMode(VIEW_WIDTH, VIEW_HEIGHT, 32, SDL_SWSURFACE)) == NULL) {
				printf("failed to initialize videomode.\n");
				return -1;
		}
		sprintf(title, "%d", clientID);
		SDL_WM_SetCaption(title,NULL);

		SDL_FillRect(gMainWindow,NULL,0xffffff);

		SDL_Flip(gMainWindow);

		return 0;
}

int drawWindow()
{
		int endFlag = 1;
		return endFlag;
}

void destroyWindow(void)
{
		SDL_Quit();
}

void windowEvent(int num)
{
		SDL_Event event;

		assert(0<num && num<=MAX_CLIENTS);
}
