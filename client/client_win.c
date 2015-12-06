#include <SDL/SDL.h>
#include <SDL/SDL_image.h>
#include <SDL/SDL_gfxPrimitives.h>
#include <SDL/SDL_ttf.h>
#include <SDL/SDL_rotozoom.h>
#include <math.h>
#include "../common.h"
#include "client_common.h"
#include "client_func.h"



#define WINDOW_WIDTH	960
#define WINDOW_HEIGHT	540


#define REACTION_VALUE	0x6fff


/*画像ファイルパス*/
static char gMapImgFile[] = "IMG/Field.png";
static char gObstacleImgFile[] = "IMG/obstacle.png";
static char gItemBoxImgFile[] = "IMG/Itembox.png";
static char BoostImgFile[] = "IMG/boost.png";
static char gItemImgFile[ITEM_NUM][20] = {
		"IMG/noizing.png",
		"IMG/Laser.png", 
		"IMG/missile.png",
		"IMG/minimum.png",
		"IMG/barrier.png"
};
static char gCharaImgFile[MAX_CLIENTS][20] ={
		"IMG/1Pship.png",
		"IMG/2Pship.png",
		"IMG/3Pship.png",
		"IMG/4Pship.png"
};
static char gIconImgFile[MAX_CLIENTS][20] = {
		"IMG/1Picon.png", 
		"IMG/2Picon.png", 
		"IMG/3Picon.png", 
		"IMG/4Picon.png"
};
static char gNameImgFile[MAX_CLIENTS][20] = {
		"IMG/1Pname.png",
		"IMG/2Pname.png",
		"IMG/3Pname.png",
		"IMG/4Pname.png"
};
static char gArrowImgFile[MAX_CLIENTS][20] = {
		"IMG/1Parrow.jpg",
		"IMG/2Parrow.jpg",
		"IMG/3Parrow.jpg",
		"IMG/4Parrow.jpg"
};

static int hiritu = 100;
static int weitFlag = 0;
static int myID;
static POSITION center;

/* ジョイスティック */
SDL_Joystick* joystick;

/*サーフェース*/
static SDL_Surface *gMainWindow;//メインウィンドウ
static SDL_Surface *gStatusWindow; //各プレイヤーのステータスウィンドウ
static SDL_Surface *gWorld;//背景画像のサーフェス
static SDL_Surface *gItemImage[ITEM_NUM];//アイテム
static SDL_Surface *gCharaImage[MAX_CLIENTS];//プレイヤー
static SDL_Surface *ObstacleImage[1]; //障害物
static SDL_Surface *gIconImage[MAX_CLIENTS];//アイコン
static SDL_Surface *gArrowImage[MAX_CLIENTS]; //矢印
static SDL_Surface *gItemBox; //アイテム欄
static SDL_Surface *gNameImage[MAX_CLIENTS]; //キャラクター名
static SDL_Surface *BoostImage; //ブースト

/*関数*/
static void drawObject();
static void drawStatus();
static int initImage();
static void combineWindow(POSITION* myPos);
static void adjustWindowPosition(SDL_Rect* windowPos, POSITION* pos);
static void clearWindow();
static int judgeRange(POSITION *objPos, POSITION *myPos);
static int measureDist(POSITION *objPos, POSITION *myPos);



typedef struct {
		SDL_Rect src;
		SDL_Rect dst;
} Rect;


int initWindows(int clientID, int num) { //ウィンドウ生成
		int i;
		char *s, title[10];
		myID = clientID;

		assert(0<num && num<=MAX_CLIENTS);
		if (initImage() < 0) {//画像の読み込み
				printf("failed to load image.\n");
				return -1;
		}

		if (SDL_Init(SDL_INIT_EVERYTHING) < 0) {
				printf("failed to initialize SDL.\n");
				return -1;
		}
		//MainWindow
		if ((gMainWindow = SDL_SetVideoMode(WINDOW_WIDTH, WINDOW_HEIGHT, 32, SDL_SWSURFACE)) == NULL) {
				printf("failed to initialize videomode.\n");
				return -1;
		}
		//StatusWindow
		if((gStatusWindow = SDL_CreateRGBSurface(SDL_SWSURFACE, gItemBox->w*4, gItemBox->h, 32, 0, 0, 0, 0)) == NULL) {
				printf("failed to initialize statuswindow");
				return -1;
		}


		sprintf(title, "%d", clientID);
		SDL_WM_SetCaption(title,NULL);

		SDL_FillRect(gMainWindow,NULL,0xffffff);

		SDL_Flip(gMainWindow);


		// initalize Joystick
		SDL_JoystickEventState(SDL_ENABLE);
		if (SDL_NumJoysticks() > 0) {
				joystick = SDL_JoystickOpen(0);
		} else {
				fprintf(stderr, "Failed to connect joystick! Eror: %s\n", SDL_GetError());
				return -1;
		}

		return 0;
}

int drawWindow() {	//ゲーム画面の描画
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
		SDL_FreeSurface (gStatusWindow);
		SDL_FreeSurface (gWorld);
		SDL_FreeSurface (ObstacleImage[0]);
		SDL_FreeSurface (gItemBox);
		int i;
		for(i = 0; i < ITEM_NUM; i++){
				SDL_FreeSurface (gItemImage[i]);
		}
		for(i = 0; i < MAX_CLIENTS; i++){
				SDL_FreeSurface (gCharaImage[i]);
				SDL_FreeSurface (gIconImage[i]);
		}

		if (joystick != NULL)
				SDL_JoystickClose(joystick);

		SDL_Quit();
}


/*
 *	入力イベント操作
 *	return: error = 0
 */
int windowEvent() {
		SDL_Event event;
		int endFlag = 1;

		// ループ内のイベントを全て所得（ジョイスティックの値が蓄積しているため）
		while (SDL_PollEvent(&event)) {
				switch (event.type) { 
						case SDL_JOYAXISMOTION: //方向キーorアナログスティック
						 		break;
						case SDL_JOYBUTTONDOWN: //ボタンが押された時
#ifndef NDEBUG
								printf("press buton: %d\n", event.jbutton.button);
#endif
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

		// get analog stick method
		Sint16 xValue = SDL_JoystickGetAxis(joystick, 0);
		Sint16 yValue = SDL_JoystickGetAxis(joystick, 1);
		double range = pow(xValue, 2) + pow(yValue, 2);
		if (range > pow(REACTION_VALUE, 2)) {	// 一定以上の角度で反応
				rotateTo(xValue, yValue);
#ifndef NDEBUG
				printf("joystick valule[x: %6d, y: %6d]\n", xValue, yValue);
#endif
		} else
				fixRotation();

		return endFlag;
}


/********* static *************/

int initImage(void){ //画像の読み込み
		int i;
		gWorld = IMG_Load( gMapImgFile ); //マップ画像 
		if ( gWorld == NULL ) {
				printf("not find world image\n");
				return(-1);
		}
		ObstacleImage[0] = IMG_Load( gObstacleImgFile ); //障害物
		if( ObstacleImage[0] == NULL ){
				printf("not find obstacle image\n");
				return(-1);
		}
		BoostImage = IMG_Load( BoostImgFile ); //ブースト
		if( BoostImage == NULL ){
				printf("not find boost image\n");
				return(-1);
		}
		gItemBox = IMG_Load( gItemBoxImgFile ); //アイテムボックス
		if( gItemBox == NULL){
				printf("not find itembox image\n");
				return(-1);
		}
		for(i = 0; i < ITEM_NUM; i++){ //アイテム画像
				gItemImage[i] = IMG_Load( gItemImgFile[i] );
				if( gItemImage[i] == NULL ){
						printf("not find item%dimage\n", i+1);
						return(-1);
				}
		}
		for(i = 0; i < MAX_CLIENTS; i++){ //キャラクター画像
				gCharaImage[i] = IMG_Load( gCharaImgFile[i] );
				if( gCharaImage[i] == NULL ){
						printf("not find chara%dimage\n", i+1);
						return(-1);
				}
		}
		for(i = 0; i < MAX_CLIENTS; i++){ //アイコン画像
				gIconImage[i] = IMG_Load( gIconImgFile[i] );
				if( gIconImage[i] == NULL ){
						printf("not find icon%d image\n", i+1);
						return(-1);
				}
		}
		for(i = 0; i < MAX_CLIENTS; i++){ //アイコン画像
				gNameImage[i] = IMG_Load( gNameImgFile[i] );
				if( gNameImage[i] == NULL ){
						printf("not find name%dP image\n", i+1);
						return(-1);
				}
		}
		for(i = 0; i < MAX_CLIENTS; i++){ //矢印画像
				gArrowImage[i] = IMG_Load( gArrowImgFile[i] );
				if( gArrowImage[i] == NULL ){
						printf("not find arrow%dP image\n", i+1);
						return(-1);
				}
		}
}


void clearWindow(void){ //ウィンドウのクリア

	//メインウィンドウ
	POSITION* myPos = &myPlayer->object->pos;
	int asp = 30;
	SDL_FillRect(gMainWindow, NULL, 0x000000); // 白で塗り潰し
	//背景を貼り付ける
	Rect ground;
	POSITION bgPos;
	bgPos.x = myPos->x/asp;
	bgPos.y = myPos->y/asp;
	int diffWidth = (gWorld->w - WINDOW_WIDTH) / 2;
	int diffHeight = (gWorld->h - WINDOW_HEIGHT) / 2;
	ground.src.x = diffWidth + bgPos.x;
	ground.src.y = diffHeight + bgPos.y;
	ground.src.w = gMainWindow->w;
	ground.src.h = gMainWindow->h;
	ground.dst.x = 0;
	ground.dst.y = 0;
	SDL_BlitSurface(gWorld, &ground.src, gMainWindow, &ground.dst);


	//ステータスウィンドウ
	SDL_Rect src_rect = {0, 0, gItemBox->w, gItemBox->h};
	SDL_Rect dst_rect = {0, 0};
	int i;
	for(i=0; i < MAX_CLIENTS; i++){
	  dst_rect.x = i*(gItemBox->w);
	  SDL_BlitSurface(gItemBox, &src_rect, gStatusWindow, &dst_rect);
	  }

}


void drawObject(void) { //オブジェクトの描画
	SDL_Rect src_rect;
	SDL_Rect dst_rect;
	SDL_Surface *image_reangle;
	double angle,ar_angle;
	int i;
	int chara_id, item_id, obstacle_id, enemy_id;
	POSITION objPos;
	src_rect.x = 0;
	src_rect.y = 0;
	POSITION* myPos = &myPlayer->object->pos;
	POSITION diffPos;
	static SDL_Surface *chara;

		int rmask, gmask, amask, bmask;
	for(i=0; i<MAX_OBJECT; i++){
	    if(judgeRange(&object[i].pos, myPos) > 0){ //表示範囲内にあれば
		switch(object[i].type){
		  case OBJECT_CHARACTER: //キャラクター
		rmask = 0xff000000;
		gmask = 0x00ff0000;
		bmask = 0x0000ff00;
		amask = 0x000000ff;
			chara = SDL_CreateRGBSurface(SDL_SWSURFACE, WINDOW_WIDTH, WINDOW_HEIGHT, 32, rmask, gmask, bmask, amask);
			SDL_SetColorKey(chara, SDL_SRCCOLORKEY, SDL_MapRGB(chara->format, 0x00, 0x00, 0x00)); //黒を透過
chara = SDL_DisplayFormat(chara);

			chara_id = ((PLAYER*)object[i].typeBuffer)->num; //キャラ番号
			angle = player[chara_id].dir * HALF_DEGRESS / PI; //キャラの向き
			image_reangle = rotozoomSurface(gCharaImage[chara_id], angle, 1.0, 1); //角度の変更
			src_rect.w = image_reangle->w;
			src_rect.h = image_reangle->h;
			int dx = image_reangle->w - gCharaImage[chara_id]->w; //回転によるずれの調整差分
			int dy = image_reangle->h - gCharaImage[chara_id]->h;
			diffPos.x = object[i].pos.x - myPos->x - (gCharaImage[chara_id]->w /2) - dx/2;
			diffPos.y = object[i].pos.y - myPos->y - (gCharaImage[chara_id]->h /2) - dy/2;
			adjustWindowPosition(&dst_rect, &diffPos);
			SDL_BlitSurface(image_reangle, &src_rect, chara, &dst_rect);
			SDL_Rect chara_rect = {0, 0, WINDOW_WIDTH, WINDOW_HEIGHT};
			dst_rect.x = 0;
			dst_rect.y = 0;
			SDL_BlitSurface(chara, &chara_rect, gMainWindow, &dst_rect);
			break;

		  case OBJECT_ITEM: //アイテム
		  	item_id = object[i].id;
			src_rect.w = gItemImage[item_id]->w;
			src_rect.h = gItemImage[item_id]->h;
			diffPos.x = object[i].pos.x - myPos->x - gItemImage[item_id]->w/2;
			diffPos.y = object[i].pos.y - myPos->y - gItemImage[item_id]->h/2;
			adjustWindowPosition(&dst_rect, &diffPos);
			SDL_BlitSurface(gItemImage[item_id], &src_rect, gMainWindow, &dst_rect);
			break;
			
		  case OBJECT_OBSTACLE: //障害物
			src_rect.w = ObstacleImage[0]->w;
			src_rect.h = ObstacleImage[0]->h;
			diffPos.x = object[i].pos.x - myPos->x - (ObstacleImage[0]->w /2);
			diffPos.y = object[i].pos.y - myPos->y - (ObstacleImage[0]->h /2);
			adjustWindowPosition(&dst_rect, &diffPos);
			SDL_BlitSurface(ObstacleImage[0], &src_rect, gMainWindow, &dst_rect);
			break;

		  case OBJECT_EMPTY: //なし
			break;
		  }
	    } else if(object[i].type == OBJECT_CHARACTER){ //敵の方向に矢印を表示
		  enemy_id = object[i].id;
		  double x,y;
		  double ar_x, ar_y;
		  x = object[i].pos.x - myPos->x;
		  y = object[i].pos.y - myPos->y;
		  ar_angle = atan2(y,x); //角度を求める
		  image_reangle = rotozoomSurface(gArrowImage[enemy_id], ar_angle, 1.0, 1); //角度の変更
		  int dx = image_reangle->w - gArrowImage[enemy_id]->w; //回転によるずれの調整差分
		  int dy = image_reangle->h - gArrowImage[enemy_id]->h;
		  ar_x = (WINDOW_HEIGHT/2)*cos(ar_angle); //矢印の位置を決める
		  ar_y = (WINDOW_HEIGHT/2)*sin(ar_angle);
		  diffPos.x = ar_x - (gArrowImage[enemy_id]->w /2) - dx/2;
		  diffPos.y = ar_y - (gArrowImage[enemy_id]->h /2) - dy/2;
		  adjustWindowPosition(&dst_rect, &diffPos);
		  SDL_BlitSurface(image_reangle, &src_rect, gMainWindow, &dst_rect);
	    }
	}
	if(image_reangle != NULL){
		SDL_FreeSurface(image_reangle);
	}
	if(chara != NULL){
		SDL_FreeSurface(chara);
	}
}


static void adjustWindowPosition(SDL_Rect* windowPos, POSITION* pos) {
		int diffWidth = WINDOW_WIDTH / 2; //y軸の中心
		int diffHeight = WINDOW_HEIGHT / 2; //x軸の中心
		windowPos->x = diffWidth + pos->x;
		windowPos->y = diffHeight + pos->y;
}


void drawStatus(void){ //ステータスの描画
		SDL_Rect src_rect;
		SDL_Rect dst_rect;
		src_rect.x = 0;
		src_rect.y = 0;
		int i;
		int item_id;
		int chara_id;

		for(i=0; i<MAX_CLIENTS; i++){

		    chara_id = player[i].num;	// キャラ番号
		    item_id = player[i].item;	// アイテム番号
		    //アイコン
		    src_rect.w = gIconImage[chara_id]->w;
		    src_rect.h = gIconImage[chara_id]->h;
		    dst_rect.x = chara_id*gItemBox->w + (gItemBox->w/2 - gIconImage[chara_id]->w)/2;
		    dst_rect.y = (gItemBox->h - gIconImage[chara_id]->h)/2;
		    SDL_BlitSurface(gIconImage[chara_id], &src_rect, gStatusWindow, &dst_rect);
		    //所持アイテム
		    if(item_id != ITEM_EMPTY){
			src_rect.w = gItemImage[item_id]->w;
			src_rect.h = gItemImage[item_id]->h;
			dst_rect.x += gItemBox->w/2;
			SDL_BlitSurface(gItemImage[item_id], &src_rect, gStatusWindow, &dst_rect);
		    }
		}
		SDL_Flip(gStatusWindow);//描画更新
}


void combineWindow(POSITION* myPos){ //サーフェスを合体
		
		//ステータスを組み合わせる
		Rect status;
		status.src.x = 0;
		status.src.y = 0;
		status.src.w = gStatusWindow->w;
		status.src.h = gStatusWindow->h;
		status.dst.x = 0;
		status.dst.y = gMainWindow->h - gStatusWindow->h;
		SDL_BlitSurface(gStatusWindow, &status.src, gMainWindow, &status.dst);
}



int judgeRange(POSITION *objPos, POSITION *myPos)
{ //範囲内にオブジェクトがあるか判定
		int range_w = WINDOW_WIDTH/2 + 200;
		int range_h = WINDOW_HEIGHT/2 + 200;
		if(objPos->x > (myPos->x - range_w)
			&& objPos->x < (myPos->x + range_w)){ //横
			if(objPos->y > (myPos->y - range_h)
				&& objPos->y < (myPos->y + range_h)){ //縦
				return 1;
			}
		}
		return -1;
}
	
			



		



