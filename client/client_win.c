#include <SDL/SDL.h>
#include <SDL/SDL_image.h>
#include <SDL/SDL_gfxPrimitives.h>
#include <SDL/SDL_ttf.h>
#include "../common.h"
#include "client_common.h"
#include "client_func.h"

#define VIEW_WIDTH	1280
#define VIEW_HEIGHT	720

#define REACTION_VALUE	0x3fff

static int weitFlag = 0;
static int myID;

static SDL_Surface *gMainWindow;

typedef struct {
		SDL_Rect src;
		SDL_Rect dst;
} Rect;


int initWindows(int clientID, int num)
{
		int i;
		char *s, title[10];
		myID = clientID;

		assert(0<num && num<=MAX_CLIENTS);

		if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_JOYSTICK) < 0) {
				printf("failed to initialize SDL.\n");
				return -1;
		}

		if ((gMainWindow = SDL_SetVideoMode(VIEW_WIDTH, VIEW_HEIGHT, 32, SDL_SWSURFACE)) == NULL) {
				printf("failed to initialize videomode.\n");
				return -1;
		}
		sprintf(title, "%d", clientID);
		SDL_WM_SetCaption(title,NULL);

		SDL_FillRect(gMainWindow,NULL,0xffffff);

		SDL_Flip(gMainWindow);

		return 0;
}

int drawWindow()//ここを主に編集
{
		int endFlag = 1;

		gMapImage = IMG_Load( gMapImgFile ); //マップデータ読み込み
		if ( gMapImage == NULL ) {
				printf("Failedreadmapimg\n");//読み込めない時のエラー表示
				exit(-1);
		}

		SDL_Surface* image;
		Rect fieldRect;

		image = SDL_LoadBMP("Field.bmp");

		fieldRect.src.x = 0;
		fieldRect.src.y = 0;
		fieldRect.src.w = image->w;
		fieldRect.src.h = image->h;

		fieldRect.dst.x = 0;
		fieldRect.dst.y = 0;

		SDL_BlitSurface(image, &rect, gMainWindow, &scr_rect);	
		SDL_BlitSurface(image, &(fieldRect.src), gMainWindow, &(fieldRect.dst));
		SDL_Flip(gMainWindow);//描画更新

		return endFlag; //endflagは1で返す(継続)
}


void destroyWindow(void) {
		SDL_Quit();
}


/*
 *	ジョイパッド操作
 */
void windowEvent(int num) {
		SDL_Event event;
		assert(0<num && num<=MAX_CLIENTS);

		if (SDL_PollEvent(&event)) {	// イベント所得
				switch(event.type) { 
						case SDL_JOYAXISMOTION: //方向キーorアナログスティック
								int axis = event.jaxis.axis;
								int value = event.jaxis.value;

								if (axis == 0) {	// 左右)
										if (value < -REACTION_VALUE) {	// left
												changeDir(LEFT);
										} else if (value > REACTION_VALUE) {	// right
												changeDir(RIGHT);
										}
								} else if (axis == 1) {	// 上下
										if (value < -REACTION_VALUE) {	// down
												changeDir(DOWN);
										} else if (value > REACTION_VALUE) {	// up
												changeDir(UP);
										}
								}
								break;

						case SDL_JOYBUTTONDOWN: //ボタンが押された時
								switch(event.jbutton.button) { //ボタン毎の処理
										case 2: //ジェット噴射
												boost(GO); //速度上昇
												break;
										case 4: //ジェット逆噴射
												boost(BACK); //速度下降（逆方向にブースト）
												break;
								}
								break;

						case SDL_JOYBUTTONUP: //ボタンが離された時
								switch (event.jbutton.button) {
										case 2:
										case 4:
												boost(NEWTRAL);	// 徐々に減速
												break;
										case 6:	//アイテム使用
												ItemUse();
												break;
								}
								break;
						default:
								break;
				}
		}

}
