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


		switch(SDL_PollEvent(&event)){ //イベント取得
			case SDL_JOYAXISMOTION: //方向キーorアナログスティック
			
			   break;

			case SDL_JOYBUTTONDOWN: //ボタンが押された時
			   switch(event.jbutton.button){ //ボタン毎の処理
			      case 2: //ジェット噴射
			         player.ver.vx++; //速度上昇（ブースト）
			         player.ver.vy++;
			         break;
			      case 4: //ジェット逆噴射
			         player.ver.vx--; //速度下降（逆方向にブースト）
			         player.ver.vy--;
			         break;
			   }
			   break;


			case SDL_JOYBUTTONUP: //ボタンが離された時
			   if(event.jbutton.button == 6){ //アイテム使用
			      ItemUse();
			}
			   //徐々に減速する
			   if(player.ver.vx > 0){
			      player.ver.vx--;
			      player.ver.vy--;
			   }
			   if(player.ver.vx < 0){
			      player.ver.vx++;
			      player.ver.vy++;
			   }
			   break;
		}

}








