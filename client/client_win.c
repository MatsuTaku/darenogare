#include <SDL/SDL.h>
#include <SDL/SDL_image.h>
#include <SDL/SDL_gfxPrimitives.h>
#include <SDL/SDL_ttf.h>
#include <SDL/SDL_rotozoom.h>
#include "../common.h"
#include "client_common.h"
#include "client_func.h"

#define VIEW_WIDTH	1280
#define VIEW_HEIGHT	720
    

#define REACTION_VALUE	0x3fff

/*画像ファイルパス*/
static char gMapImgFile[] = "IMG/Field.png"; //背景画像
static char ObstacleImgFile[] = "IMG/obstacle.png"; //隕石画像
static char Item1ImgFile[] = "IMG/Thunder.png";
static char Item2ImgFile[] = "IMG/";
static char Item3ImgFile[] = "IMG/";
static char Item4ImgFile[] = "IMG/";
static char Item5ImgFile[] = "IMG/";
static char gChara1ImgFile[] = "IMG/1Pship.png"; //操作キャラ画像
static char gChara2ImgFile[] = "IMG/2Pship.png";
static char gChara3ImgFile[] = "IMG/3Pship.png";
static char gChara4ImgFile[] = "IMG/4Pship.png";
static char gIcon1ImgFile[] = "IMG/1Picon.png"; //キャラのアイコン
static char gIcon2ImgFile[] = "IMG/2Picon.png";
static char gIcon3ImgFile[] = "IMG/3Picon.png";
static char gIcon4ImgFile[] = "IMG/4Picon.png";
static char gItemBoxImgFile[] = "IMG/Itembox.png"; //アイテム欄

static int weitFlag = 0;
static int myID;
static POSITION mypos;

/*サーフェース*/
static SDL_Surface *gMainWindow;//メインウィンドウ
static SDL_Surface *gWorldWindow; //各プレイヤーのマップウィンドウ
static SDL_Surface *gStatusWindow; //各プレイヤーのステータスウィンドウ
static SDL_Surface *gWorld;//背景画像
static SDL_Surface *gItemImage[ITEM_NUM];//アイテム
static SDL_Surface *gCharaImage[CT_NUM];//プレイヤー
static SDL_Surface *ObstacleImage[1]; //障害物
static SDL_Surface *gIconImage[CT_NUM];//アイコン
static SDL_Surface *gItemBox; //アイテム欄

/*関数*/
static void drawObject();
static void drawStatus();
static int initImage();
static void combineWindow();
static void clearWindow();


OBJECT allObject[MAX_OBJECT];
PLAYER allPlayer[MAX_CLIENTS];

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
		if (initImage() < 0){//画像の読み込み
				printf("failed to load image.\n");
				return -1;
		}

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


		clearWindow(); //ウィンドウのクリア
		drawObject(); //オブジェクトの描画
		drawStatus(); //ステータスの描画
		combineWindow(); //画面の切り抜き

		SDL_Flip(gMainWindow);//描画更新

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
								double range = pow(xValue, 2) + pow(yValue, 2);
								// if (range > pow(REACTION_VALUE, 2))
								rotateTo(xValue, yValue);
#ifndef NDEBUG
								printf("joystick valule[x: %6d, y: %6d]\n", xValue, yValue);
#endif
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



/**** static *****/
int initImage(void){ //画像の読み込み
		gWorld = IMG_Load( gMapImgFile ); 
		if ( gWorld == NULL ) {
			printf("not find world image\n");
			return(-1);
		}
		gItemImage[0] = IMG_Load( Item1ImgFile );
		if( gItemImage[0] == NULL ){
			printf("not find item1 image\n");
			return(-1);
		}
		gItemImage[1] = IMG_Load( Item2ImgFile );
		if( gItemImage[1] == NULL ){
			printf("not find item2 image\n");
			return(-1);
		}
		gItemImage[2] = IMG_Load( Item3ImgFile );
		if( gItemImage[2] == NULL ){
			printf("not find item3 image\n");
			return(-1);
		}
		gItemImage[3] = IMG_Load( Item4ImgFile );
		if( gItemImage[3] == NULL ){
			printf("not find item4 image\n");
			return(-1);
		}
		gItemImage[4] = IMG_Load( Item1ImgFile );
		if( gItemImage[4] == NULL ){
			printf("not find item5 image\n");
			return(-1);
		}
		gWorld = IMG_Load( gMapImgFile );
		if( gWorld == NULL ){
			printf("not find world image\n");
			return(-1);
		}
		gCharaImage[0] = IMG_Load( gChara1ImgFile );
		if( gCharaImage[0] == NULL ){
			printf("not find item2 image\n");
			return(-1);
		}
		gCharaImage[1] = IMG_Load( gChara2ImgFile );
		if( gCharaImage[1] == NULL ){
			printf("not find item2 image\n");
			return(-1);
		}
		gCharaImage[2] = IMG_Load( gChara3ImgFile );
		if( gCharaImage[2] == NULL ){
			printf("not find item2 image\n");
			return(-1);
		}
		gCharaImage[3] = IMG_Load( gChara4ImgFile );
		if( gCharaImage[3] == NULL ){
			printf("not find item2 image\n");
			return(-1);
		}
		ObstacleImage[0] = IMG_Load( ObstacleImgFile );
		if( ObstacleImage[0] == NULL ){
			printf("not find item2 image\n");
			return(-1);
		}
		gIconImage[0] = IMG_Load( gIcon1ImgFile );
		if( gIconImage[0] == NULL){
			printf("not find icon image\n");
			return(-1);
		}
		gIconImage[1] = IMG_Load( gIcon2ImgFile );
		if( gIconImage[1] == NULL){
			printf("not find icon image\n");
			return(-1);
		}
		gIconImage[2] = IMG_Load( gIcon3ImgFile );
		if( gIconImage[2] == NULL){
			printf("not find icon image\n");
			return(-1);
		}
		gIconImage[3] = IMG_Load( gIcon4ImgFile );
		if( gIconImage[3] == NULL){
			printf("not find icon image\n");
			return(-1);
		}
}


void clearWindow(void){ //ウィンドウのクリア

	//ワールドウィンドウ
	SDL_Rect src_rect = {0, 0, gWorld->w, gWorld->h};
	SDL_Rect dst_rect = {0, 0};
	SDL_BlitSurface(gWorld, &src_rect, gWorldWindow, &dst_rect);

	//ステータスウィンドウ
	src_rect.w = gItemBox->w;
	src_rect.h = gItemBox->h;
	int i;
	for(i=0; i < CT_NUM; i++){
	  dst_rect.x = i*(gItemBox->w);
	  SDL_BlitSurface(gStatusWindow, &src_rect, gStatusWindow, &dst_rect);
	  }
}


void drawObject(void){ //オブジェクトの描画
	SDL_Rect src_rect;
	SDL_Rect dst_rect;
	SDL_Surface *image_reangle;
	double angle;
	int i;
	int chara_id, item_id, obstacle_id;
	src_rect.x = 0;
	src_rect.y = 0;

	for(i=0; i<MAX_OBJECT; i++){
		switch(allObject[i].type){

		  case OBJECT_CHARACTER: //キャラクター
			chara_id = allObject[i].id; //キャラ番号
			if(chara_id == myID){ //自分の居場所を保存
			  mypos.x = allObject[i].pos.x;
			  mypos.y = allObject[i].pos.y;
			}
			angle = allPlayer[chara_id].dir; //キャラの向き
			src_rect.w = gCharaImage[chara_id]->w;
			src_rect.h = gCharaImage[chara_id]->h;
			image_reangle = rotozoomSurface(gCharaImage[chara_id], angle, 1.0, 1); //角度の変更
			int dx = image_reangle->w - src_rect.w; //回転によるずれの調整差分
			int dy = image_reangle->h - src_rect.h;
			dst_rect.x = allObject[i].pos.x - (gCharaImage[chara_id]->w /2) - dx/2;
			dst_rect.y = allObject[i].pos.y - (gCharaImage[chara_id]->h /2) - dy/2; 
			SDL_BlitSurface(image_reangle, &src_rect, gWorldWindow, &dst_rect);
			break;

		  case OBJECT_ITEM: //アイテム
		  	item_id = allObject[i].id;
			src_rect.w = gItemImage[item_id]->w;
			src_rect.h = gItemImage[item_id]->h;
			dst_rect.x = allObject[i].pos.x - (gItemImage[item_id]->w /2);
			dst_rect.y = allObject[i].pos.y - (gItemImage[item_id]->h /2);
			SDL_BlitSurface(gItemImage[item_id], &src_rect, gWorldWindow, &dst_rect);
			break;
			
		  case OBJECT_OBSTACLE: //障害物
			obstacle_id = allObject[i].id;
			src_rect.w = ObstacleImage[obstacle_id]->w;
			src_rect.h = ObstacleImage[obstacle_id]->h;
			dst_rect.x = allObject[i].pos.x - (ObstacleImage[obstacle_id]->w /2);
			dst_rect.y = allObject[i].pos.y - (ObstacleImage[obstacle_id]->h /2);
			SDL_BlitSurface(ObstacleImage[obstacle_id], &src_rect, gWorldWindow, &dst_rect);
			break;

		  case OBJECT_EMPTY: //なし
			break;
		  }
	}
}

void drawStatus(void){ //ステータスの描画
		SDL_Rect src_rect;
		SDL_Rect dst_rect;
		src_rect.x = 0;
		src_rect.y = 0;
		dst_rect.y = 50;

		int i;
		int item_id;
		int chara_id;

		for(i=0; i<4; i++){
		    chara_id = allPlayer[i].id;
		    item_id = allPlayer[i].item;
		    //アイコン
		    src_rect.w = gIconImage[chara_id]->w;
		    src_rect.h = gIconImage[chara_id]->h;
		    dst_rect.x = chara_id*(gItemBox->w);
		    SDL_BlitSurface(gIconImage[chara_id], &src_rect, gStatusWindow, &dst_rect);
		    //所持アイテム
		    if(item_id == 0) break;
		    src_rect.w = gItemImage[item_id]->w;
		    src_rect.h = gItemImage[item_id]->h;
		    dst_rect.x += 50;
		    SDL_BlitSurface(gItemImage[item_id], &src_rect, gStatusWindow, &dst_rect);
		}
}


void combineWindow(void){ //各プレイヤーの画面の作成
		SDL_Rect src_rect;
		SDL_Rect dst_rect = {0, 0};		
		src_rect.x = mypos.x - (VIEW_WIDTH/2);
		src_rect.y = mypos.y - (VIEW_HEIGHT/2);
		src_rect.w = VIEW_WIDTH;
		src_rect.h = VIEW_HEIGHT;
		SDL_BlitSurface(gWorldWindow, &src_rect, gMainWindow, &dst_rect);
		//ここからステータスウィンドウも組み合わせる
}





