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

/*画像ファイルパス*/
static char gMapImgFile[] = "Field.bmp";
static char gObstacleImgFile[] = "obstacle.png";
static char gItemImgFile[] = "Thunder.png";

static int weitFlag = 0;
static int myID;

/*サーフェース*/
static SDL_Surface *gMainWindow;//メインウィンドウ
static SDL_Surface *gWorld;//背景画像
static SDL_Surface *gItem[ITEM_NUM];//アイテム
static SDL_Surface *gCharaImage[CT_NUM];//プレイヤー
static SDL_Surface *Obstacle;//障害物


typedef struct {
		SDL_Rect src;
		SDL_Rect dst;
} Rect;

OBJECT object;


int initWindows(int clientID, int num) //ウィンドウ生成
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

int drawWindow()//ゲーム画面の描画
{
		int endFlag = 1;

		gWorld = IMG_Load( gMapImgFile ); 
		if ( gWorld == NULL ) {
				printf("Failedreadmapimg\n");//読み込めない時のエラー表示
				exit(-1);
		}
		SDL_Surface* image;
		Rect fieldRect;
		image = SDL_LoadBMP("Field.bmp");//背景読み込み

		fieldRect.src.x = 0;
		fieldRect.src.y = 0;
		fieldRect.src.w = image->w;
		fieldRect.src.h = image->h;
		fieldRect.dst.x = 0;
		fieldRect.dst.y = 0;

		/*アイテム欄の生成(黒で塗りつぶし)(1P,2P,3P,4P)*/
		boxColor(gWorld,130,540,190,600,0xffffff);    
		boxColor(gWorld,450,540,510,600,0xffffff);                            
		boxColor(gWorld,770,540,830,600,0xffffff); 
		boxColor(gWorld,1090,540,1150,600,0xffffff);                          


		SDL_BlitSurface(image, &(fieldRect.src), gWorld, &(fieldRect.dst));
		SDL_Flip(gWorld);//描画更新

		return endFlag; //endflagは1で返す(継続)
}


void destroyWindow(void) {
		SDL_Quit();
}


/*
 *	入力イベント操作
 *	return: error = 0
 */
int windowEvent() {
		SDL_Event event;
		int endFlag = 1;
		SDL_Joystick* joystick;

		if (SDL_PollEvent(&event)) {	// イベント所得
				switch(event.type) { 
						case SDL_JOYAXISMOTION: //方向キーorアナログスティック
								joystick = SDL_JoystickOpen((int)event.jaxis.which);
								Sint16 xValue = SDL_JoystickGetAxis(joystick, 0);
								Sint16 yValue = SDL_JoystickGetAxis(joystick, 1);
#ifndef NDEBUG
								//printf("joypad axis[%1d]	value[%6d]\n", event.jaxis.axis, event.jaxis.value);
								printf("joystick valule[x: %6d, y: %6d]\n", xValue, yValue);
#endif
								if (event.jaxis.axis == 0) {	// 左右)
										if (event.jaxis.value < -REACTION_VALUE) {	// left
												rotateLeft();
										} else if (event.jaxis.value > REACTION_VALUE) {	// right
												rotateRight();
										} else {
												fixRotation();
										}
								} else if (event.jaxis.axis == 1) {	// 上下
										if (event.jaxis.value < -REACTION_VALUE) {	// down
										} else if (event.jaxis.value > REACTION_VALUE) {	// up
										} else {
										}
								}
								break;

						case SDL_JOYBUTTONDOWN: //ボタンが押された時
								switch(event.jbutton.button) { //ボタン毎の処理
										case BUTTON_CIRCLE: //ジェット噴射
												//速度上昇
												acceleration();
												break;
										case BUTTON_CROSS: //ジェット逆噴射
												//速度下降（逆方向にブースト）
												deceleration();
												break;
								}
								break;

						case SDL_JOYBUTTONUP: //ボタンが離された時
								switch (event.jbutton.button) {
										case BUTTON_CIRCLE:
										case BUTTON_CROSS:
												inertialNavigation();
										case BUTTON_TRIANGLE:	//アイテム使用
												useItem();
												break;
										default:
												break;
								}
								break;
						case SDL_QUIT:
								endFlag = 0;
#ifndef NDEBUG
								printf("Press close button\n");
#endif
								break;
						default:
								break;
				}
		}

		return endFlag;
}
