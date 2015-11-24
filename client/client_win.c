#include <SDL/SDL.h>
#include <SDL/SDL_image.h>
#include <SDL/SDL_gfxPrimitives.h>
#include <SDL/SDL_ttf.h>
#include <SDL/SDL_rotozoom.h>
#include "../common.h"
#include "client_common.h"
#include "client_func.h"

#define VIEW_WIDTH	640
#define VIEW_HEIGHT	360
    

#define REACTION_VALUE	0x3fff

/*画像ファイルパス*/
static char gMapImgFile[] = "IMG/Field.png"; //背景画像
static char ObstacleImgFile[] = "IMG/obstacle.png"; //隕石画像
static char Item1ImgFile[] = "IMG/noizing.png";
static char Item2ImgFile[] = "IMG/Laser.png";
static char Item3ImgFile[] = "IMG/missile.png";
static char Item4ImgFile[] = "IMG/minimum.png";
static char Item5ImgFile[] = "IMG/barrier.png";
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
static void combineWindow(POSITION* myPos);
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
		//MainWindow
		if ((gMainWindow = SDL_SetVideoMode(VIEW_WIDTH, VIEW_HEIGHT, 32, SDL_SWSURFACE)) == NULL) {
				printf("failed to initialize videomode.\n");
				return -1;
		}

		Uint32 rMask, gMask, bMask, aMask;
#if SDL_BYTEORDER == SDL_BIG_ENDIAN
		rMask = 0xff000000;
		gMask = 0x00ff0000;
		bMask = 0x0000ff00;
		aMask = 0x000000ff;
#else
		rMask = 0x000000ff;
		gMask = 0x0000ff00;
		bMask = 0x00ff0000;
		aMask = 0xff000000;
#endif
		//WorldWindow
		if((gWorldWindow = SDL_CreateRGBSurface(SDL_SWSURFACE, gWorld->w, gWorld->h, 32, 0,0,0,0)) == NULL)
{
			printf("failed to initialize worldwindow");
			return -1;
		}
		//StatusWindow
		if((gStatusWindow = SDL_CreateRGBSurface(SDL_SWSURFACE, gItemBox->w*4, gItemBox->h, 32, rMask, gMask, bMask, aMask)) == NULL)
{
			printf("failed to initialize statuswindow");
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
		combineWindow(&myPlayer->object->pos); //サーフェスの合体

		SDL_Flip(gMainWindow);//描画更新

		return endFlag; //endflagは1で返す(継続)
}


void destroyWindow(void) {
	SDL_FreeSurface(gMainWindow);
	SDL_FreeSurface (gWorldWindow);
	SDL_FreeSurface (gStatusWindow);
	SDL_FreeSurface (gWorld);
	SDL_FreeSurface (ObstacleImage[0]);
	SDL_FreeSurface (gItemBox);
	int i;
	for(i = 0; i < ITEM_NUM; i++){
	    SDL_FreeSurface (gItemImage[i]);
	}
	for(i = 0; i < CT_NUM; i++){
	    SDL_FreeSurface (gCharaImage[i]);
	    SDL_FreeSurface (gIconImage[i]);
	}
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
								// printf("joystick valule[x: %6d, y: %6d]\n", xValue, yValue);
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
		gItemImage[4] = IMG_Load( Item5ImgFile );
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
			printf("not find chara1 image\n");
			return(-1);
		}
		gCharaImage[1] = IMG_Load( gChara2ImgFile );
		if( gCharaImage[1] == NULL ){
			printf("not find chara2 image\n");
			return(-1);
		}
		gCharaImage[2] = IMG_Load( gChara3ImgFile );
		if( gCharaImage[2] == NULL ){
			printf("not find chara3 image\n");
			return(-1);
		}
		gCharaImage[3] = IMG_Load( gChara4ImgFile );
		if( gCharaImage[3] == NULL ){
			printf("not find chara4 image\n");
			return(-1);
		}
		ObstacleImage[0] = IMG_Load( ObstacleImgFile );
		if( ObstacleImage[0] == NULL ){
			printf("not find obstacle image\n");
			return(-1);
		}
		gIconImage[0] = IMG_Load( gIcon1ImgFile );
		if( gIconImage[0] == NULL){
			printf("not find icon1 image\n");
			return(-1);
		}
		gIconImage[1] = IMG_Load( gIcon2ImgFile );
		if( gIconImage[1] == NULL){
			printf("not find icon2 image\n");
			return(-1);
		}
		gIconImage[2] = IMG_Load( gIcon3ImgFile );
		if( gIconImage[2] == NULL){
			printf("not find icon3 image\n");
			return(-1);
		}
		gIconImage[3] = IMG_Load( gIcon4ImgFile );
		if( gIconImage[3] == NULL){
			printf("not find icon4 image\n");
			return(-1);
		}
		gItemBox = IMG_Load( gItemBoxImgFile );
		if( gItemBox == NULL){
			printf("not find itembox image\n");
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
	  dst_rect.x = i*(gItemBox->w) + 10;
	  dst_rect.y = gMainWindow->h - gStatusWindow->h;
	  SDL_BlitSurface(gItemBox, &src_rect, gStatusWindow, &dst_rect);
	  }

	//メインウィンドウ
	src_rect.w = gMainWindow->w;
	dst_rect.h = gMainWindow->h;
	SDL_FillRect(gMainWindow, NULL, 0xffffff);

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
			chara_id = ((PLAYER*)allObject[i].typeBuffer)->num; //キャラ番号
			angle = allPlayer[chara_id].dir * HALF_DEGRESS / PI; //キャラの向き
			image_reangle = rotozoomSurface(gCharaImage[chara_id], angle, 1.0, 1); //角度の変更
			src_rect.x = 0;
			src_rect.y = 0;
			src_rect.w = image_reangle->w;
			src_rect.h = image_reangle->h;
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
	if(image_reangle != NULL)
	SDL_FreeSurface(image_reangle);
}

void drawStatus(void){ //ステータスの描画
		SDL_Rect src_rect;
		SDL_Rect dst_rect;
		src_rect.x = 0;
		src_rect.y = 0;
		dst_rect.y = 30;

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
		SDL_Flip(gStatusWindow);//描画更新
}


void combineWindow(POSITION* myPos){ //各プレイヤーの画面の作成
		//マップを組み合わせる
		Rect world;
		world.src.x = myPos->x - (VIEW_WIDTH/2);
		world.src.y = myPos->y - (VIEW_HEIGHT/2);
		world.src.w = VIEW_WIDTH;
		world.src.h = VIEW_HEIGHT - gStatusWindow->h;
		world.dst.x = 0;
		world.dst.y = 0;
		SDL_BlitSurface(gWorldWindow, &world.src, gMainWindow, &world.dst);
		//ステータスウィンドウも組み合わせる
		Rect status;
		status.src.x = 0;
		status.src.y = 0;
		status.src.w = gStatusWindow->w;
		status.src.h = gStatusWindow->h;
		status.dst.x = 0;
		status.dst.y = VIEW_HEIGHT - gStatusWindow->h;
		SDL_BlitSurface(gStatusWindow, &status.src, gMainWindow, &status.dst);
}





