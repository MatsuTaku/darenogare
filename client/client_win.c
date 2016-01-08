#include <SDL/SDL.h>
#include <SDL/SDL_image.h>
#include <SDL/SDL_gfxPrimitives.h>
#include <SDL/SDL_ttf.h>
#include <SDL/SDL_rotozoom.h>
#include <math.h>
#include "../common.h"
#include "client_common.h"
#include "client_func.h"

#define WINDOW_WIDTH	1000
#define WINDOW_HEIGHT	600
#define REACTION_VALUE	0x6fff


/*画像ファイルパス*/
static char gMapImgFile[] = "IMG/WallL.gif";
static char gObstacleImgFile[] = "IMG/obstacle.png";
static char gItemBoxImgFile[] = "IMG/Itembox.png";
static char gBoostImgFile[] = "IMG/boost.png";
static char gWarningImgFile[] = "IMG/warning.png";
static char gBoomImgFile[] = "IMG/boom.png";
static char gDeadIconImgFile[] = "IMG/Deadicon.png";
static char gMiniMapImgFile[] = "IMG/minimap.png";
static char gArrowImgFile[] = "IMG/target.png";
static char gLaserImgFile[2][20] = {
		"IMG/LaserEff.png",
		"IMG/LaserEff2.png"
};
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
/* フォントファイル */
static char gFontFile[] = "/usr/share/fonts/opentype/ipafont-mincho/ipamp.ttf";

/* 変数 */
static int now, interval; //アニメーション用の時間変数
static int anm_flag; //アニメーション用フラグ
static int weitFlag = 0;
static int myID;
static POSITION center;

/* フォント */
TTF_Font* font;

/* ジョイスティック */
SDL_Joystick* joystick;

/*サーフェース*/
static SDL_Surface *gMainWindow;//メインウィンドウ
static SDL_Surface *gBackGround; //背景
static SDL_Surface *gStatusWindow; //各プレイヤーのステータスウィンドウ
static SDL_Surface *gMiniMap; //ミニマップ
static SDL_Surface *gDeadIconImage; //死亡時のアイコン
static SDL_Surface *gItemImage[ITEM_NUM];//アイテム
static SDL_Surface *gCharaImage[MAX_CLIENTS];//プレイヤー
static SDL_Surface *ObstacleImage[1]; //障害物
static SDL_Surface *gIconImage[MAX_CLIENTS];//アイコン
static SDL_Surface *gArrowImage; //中心位置への方向
static SDL_Surface *gLaserImage[2]; //レーザーのエフェクト
static SDL_Surface *gItemBox; //アイテム欄
static SDL_Surface *gBoostImage; //ブースト
static SDL_Surface *gWarningImage; //警告
static SDL_Surface *gBoomImage; //爆発
static SDL_Surface *gMiniMapImage; //ミニマップ

/*関数*/
static int initImage();
static void drawObject();
static void drawChara(POSITION *charaPos, int chara_id);
static void drawBoost(int chara_id, SDL_Surface *c_window);
static void drawDeadChara(POSITION *charaPos, int chara_id);
static void drawItem(POSITION *itemPos, int item_id);
static void drawObstacle(POSITION *obsPos);
//static void drawLaser(POSITION *charaPos, int chara_id);
static void drawStatus();
static void drawWarning();
static void drawMiniMap(POSITION* myPos);
static void adjustWindowPosition(SDL_Rect* windowPos, POSITION* pos);
static void clearWindow();
static int judgeRange(POSITION *objPos, POSITION *myPos);

typedef struct {
		SDL_Rect src;
		SDL_Rect dst;
} Rect;

char playerName[MAX_CLIENTS][32] = {
		"RedFox",
		"BlueHound",
		"YellowTiger",
		"GreenDragon"
};


int initWindows(int clientID, int num) { //ウィンドウ生成
		int i;
		char *s, title[32];
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
		//MiniMap
		if((gMiniMap = SDL_CreateRGBSurface(SDL_SWSURFACE, gMiniMapImage->w, gMiniMapImage->h, 32, 0, 0, 0, 0)) == NULL) {
				printf("failed to initialize minimap");
				return -1;
		}
		if(TTF_Init() < 0){ //フォントの初期化
				printf("failed to initialize font");
				return -1;
		}

		font = TTF_OpenFont(gFontFile, 24);

		// sprintf(title, "Player No. %d", clientID + 1);
		SDL_WM_SetCaption(playerName[clientID], NULL);


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

		now = SDL_GetTicks(); //現在の時刻を取得
		if(now >= interval){ //0.1秒経ったら
			
			interval = now + 100; //次の開始時間を0.1秒後に設定
		}

		clearWindow(); //ウィンドウのクリア
		drawObject(); //オブジェクトの描画
		//drawLaser(); //レーザーの描画
		drawStatus(); //ステータスの描画
		if(myPlayer->alive){
			drawMiniMap(&myPlayer->object->pos); //ミニマップの描画
		}
		if(myPlayer->warn == WARN_OUT_AREA && myPlayer->alive){
			drawWarning(); //警告文の表示
		}
		SDL_Flip(gMainWindow);//描画更新
		return endFlag; //endflagは1で返す(継続)
}


void destroyWindow(void) { //サーフェスの解放
		SDL_FreeSurface(gMainWindow);
		SDL_FreeSurface (gStatusWindow);
		SDL_FreeSurface(gMiniMap);
		SDL_FreeSurface (gBackGround);
		SDL_FreeSurface (ObstacleImage[0]);
		SDL_FreeSurface (gItemBox);
		SDL_FreeSurface(gMiniMapImage);
		SDL_FreeSurface(gArrowImage);
		SDL_FreeSurface(gBoomImage);
		SDL_FreeSurface(gWarningImage);
		int i;
		for(i = 0; i < ITEM_NUM; i++){
				SDL_FreeSurface (gItemImage[i]);
		}
		for(i = 0; i < MAX_CLIENTS; i++){
				SDL_FreeSurface (gCharaImage[i]);
				SDL_FreeSurface (gIconImage[i]);
		}
		for(i = 0; i < 2; i++){
				SDL_FreeSurface (gLaserImage[i]);
		}

		if (joystick != NULL)
				SDL_JoystickClose(joystick);

		SDL_Quit();
}


/*
 *	入力イベント操作
 *	return: error = 0
 */
bool windowEvent() {
		SDL_Event event;
		bool endFlag = false;

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
								endFlag = true;
#ifndef NDEBUG
								printf("Press close button\n");
#endif
								return endFlag;
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
				// printf("joystick valule[x: %6d, y: %6d]\n", xValue, yValue);
#endif
		} else
				fixRotation();

		return endFlag;
}


/********* static *************/


int initImage(void){ //画像の読み込み
		int i;
		gBackGround = IMG_Load( gMapImgFile ); //マップ画像 
		if ( gBackGround == NULL ) {
				printf("not find world image\n");
				return(-1);
		}
		ObstacleImage[0] = IMG_Load( gObstacleImgFile ); //障害物
		if( ObstacleImage[0] == NULL ){
				printf("not find obstacle image\n");
				return(-1);
		}
		gBoostImage = IMG_Load( gBoostImgFile ); //ブースト
		if( gBoostImage == NULL ){
				printf("not find boost image\n");
				return(-1);
		}
		gItemBox = IMG_Load( gItemBoxImgFile ); //アイテムボックス
		if( gItemBox == NULL){
				printf("not find itembox image\n");
				return(-1);
		}
		gWarningImage = IMG_Load( gWarningImgFile ); //警告文
		if( gWarningImage == NULL){
				printf("not find warning image\n");
				return(-1);
		}
		gBoomImage = IMG_Load( gBoomImgFile ); //警告文
		if( gBoomImage == NULL){
				printf("not find boom image\n");
				return(-1);
		}
		gMiniMapImage = IMG_Load( gMiniMapImgFile ); //ミニマップ
		if( gMiniMapImage == NULL){
				printf("not find minimap image\n");
				return(-1);
		}
		gDeadIconImage = IMG_Load( gDeadIconImgFile ); //死亡時のアイコン
		if( gDeadIconImage == NULL){
				printf("not find deadicon image\n");
				return(-1);
		}
		gArrowImage = IMG_Load( gArrowImgFile ); //矢印
		if(gArrowImage == NULL){
				printf("not find arrow image\n");
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
		for(i = 0; i < 2; i++){ //レーザー画像
				gLaserImage[i] = IMG_Load( gLaserImgFile[i] );
				if( gLaserImage[i] == NULL){
						printf("not find laser%dimage\n", i+1);
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

}



void clearWindow(void){ //ウィンドウのクリア
		//メインウィンドウ
		POSITION* myPos = &myPlayer->object->pos;
		int asp = 3;
		//背景を貼り付ける
		Rect ground;
		ground.src.x = myPos->x/asp + (gBackGround->w - gMainWindow->w)/2;
		ground.src.y = myPos->y/asp + (gBackGround->h - gMainWindow->h)/2;
		ground.src.w = gMainWindow->w;
		ground.src.h = gMainWindow->h;
		ground.dst.x = 0;
		ground.dst.y = 0;
		SDL_FillRect(gMainWindow, NULL, 0x000000); //範囲外だけ黒で塗り潰し
		SDL_BlitSurface(gBackGround, &ground.src, gMainWindow, &ground.dst); //背景の描写

		//ステータスウィンドウ
		SDL_Rect src_rect = {0, 0, gItemBox->w, gItemBox->h};
		SDL_Rect dst_rect = {0, 0};
		int i;
		for(i=0; i < MAX_CLIENTS; i++){
				dst_rect.x = i*(gItemBox->w);
				SDL_BlitSurface(gItemBox, &src_rect, gStatusWindow, &dst_rect);
		}
}


static void adjustWindowPosition(SDL_Rect* windowPos, POSITION* pos) {
		int diffWidth = WINDOW_WIDTH / 2; //y軸の中心
		int diffHeight = WINDOW_HEIGHT / 2; //x軸の中心
		windowPos->x = diffWidth + pos->x;
		windowPos->y = diffHeight + pos->y;
}


void drawObject(void) { //オブジェクトの描画

	int i;
	int id;
	SDL_Rect src_rect;
	SDL_Rect dst_rect;
	POSITION diffPos;
	POSITION objPos;
	POSITION* myPos = &myPlayer->object->pos; //マイポジション
	src_rect.x = 0;
	src_rect.y = 0;

	for(i=0; i<MAX_OBJECT; i++){
	    if(judgeRange(&object[i].pos, myPos) > 0){ //表示範囲内にあれば
		switch(object[i].type){
		  case OBJECT_CHARACTER: //キャラクター
			id = ((PLAYER*)object[i].typeBuffer)->num; //キャラ番号
			if(id != myID){
				if(!player[id].alive){
					drawDeadChara(&object[i].pos, id); //死亡キャラの描画
					break;
				}
				drawChara(&object[i].pos, id); //キャラクターの描画
			}
			break;
		  case OBJECT_ITEM: //アイテム
		  	id = ((ITEM *)object[i].typeBuffer)->num; //アイテム番号
			drawItem(&object[i].pos, id); //アイテムの描画
			break;
		  case OBJECT_OBSTACLE: //障害物
			drawObstacle(&object[i].pos); //障害物の描画
			break;
		  case OBJECT_EMPTY: //なし
			break;
		  }
	    }
	}
	if(!myPlayer->alive){
		drawDeadChara(myPos, myID);
	}else{
		drawChara(myPos, myID); //プレイヤーの描画
	}
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


void drawChara(POSITION *charaPos, int chara_id){ //キャラクターの描画

		static SDL_Surface *c_window, *image_reangle; //回転後のサーフェス
		double angle,ar_angle;
		POSITION c_center;
		POSITION diffPos;
		POSITION* myPos = &myPlayer->object->pos; //マイポジション
		int dx, dy;
		int i;
		int rmask, gmask, amask, bmask;
		rmask = 0xff000000;
		gmask = 0x00ff0000;
		bmask = 0x0000ff00;
		amask = 0x000000ff;

		//c_windowの透過処理
		c_window = SDL_CreateRGBSurface(SDL_SWSURFACE, gCharaImage[chara_id]->w + gBoostImage->w + 40, gCharaImage[chara_id]->h + gBoostImage->h +60, 32, rmask, gmask, bmask, amask);
		SDL_SetColorKey(c_window, SDL_SRCCOLORKEY, SDL_MapRGB(c_window->format, 0x00, 0x00, 0x00)); //黒を透過
		c_window = SDL_DisplayFormat(c_window);
		c_center.x = c_window->w/2;
		c_center.y = c_window->h/2;

		//1.噴射炎をc_windowに描画
		drawBoost(chara_id, c_window);

		//2.特定のアイテム使用時のエフェクトをc_windowに描画
		//drawArroundEffect(アイテム使用フラグ, c_window);


		//3.キャラをc_windowに描画
		SDL_Rect src_rect = {0, 0, gCharaImage[chara_id]->w, gCharaImage[chara_id]->h};
		SDL_Rect dst_rect = {c_center.x - gCharaImage[chara_id]->w/2, c_center.y - gCharaImage[chara_id]->h/2};
		SDL_BlitSurface(gCharaImage[chara_id], &src_rect, c_window, &dst_rect);

		//4.c_windowをウインドウに描画
		angle = player[chara_id].dir * HALF_DEGRESS / PI; //キャラの向き
		if(chara_id != myID){
			angle -= 90;
		}
		image_reangle = rotozoomSurface(c_window, angle, 1.0, 1); //角度の変更
		SDL_Rect c_rect = {0, 0, image_reangle->w, image_reangle->h};
		dx = image_reangle->w - c_window->w; //回転によるずれの調整差分
		dy = image_reangle->h - c_window->h;
		diffPos.x = charaPos->x - myPos->x - (c_window->w /2) - dx/2;
		diffPos.y = charaPos->y - myPos->y - (c_window->h /2) - dy/2;
		adjustWindowPosition(&dst_rect, &diffPos);
		SDL_BlitSurface(image_reangle, &c_rect, gMainWindow, &dst_rect); //描画

		if(image_reangle != NULL){
			SDL_FreeSurface(image_reangle);
		}
		if(c_window != NULL){
			SDL_FreeSurface(c_window);
		}

}

void drawBoost(int chara_id, SDL_Surface *c_window){ //噴射炎を描画

		SDL_Surface* image_reangle;
		POSITION c_center;
		Rect boost;
		int dx, dy, i;
		int bst_flag = player[chara_id].boost; //噴射フラグ
		int rtt_flag = player[chara_id].rotate; //回転フラグ

		c_center.x = c_window->w/2;
		c_center.y = c_window->h/2;
		boost.src.x = 0; boost.src.y = 0; 
		if(bst_flag != BOOST_NEUTRAL){
		    
		    if(bst_flag == BOOST_GO){ //前噴射の場合
			for(i = 0; i < 2; i++){
				image_reangle = rotozoomSurface(gBoostImage, i*-2, 0.5, 1); //縮小
				boost.src.w = image_reangle->w; boost.src.h = image_reangle->h;
				dx = image_reangle->w - gBoostImage->w; //調整差分
				dy = image_reangle->h - gBoostImage->h;
				boost.dst.x = c_center.x - gCharaImage[chara_id]->w*1.0 -4 - dx;
				boost.dst.y = (c_center.y - image_reangle->h/2 - 1) - i*26 - dy;
				SDL_BlitSurface(image_reangle, &boost.src, c_window, &boost.dst);
			}
		    }
		    if(bst_flag == BOOST_BACK){ //後噴射の場合
			image_reangle = rotozoomSurface(gBoostImage, HALF_DEGRESS, 1.0, 1); //角度の変更
			boost.src.w = image_reangle->w; boost.src.h = image_reangle->h;
			dx = image_reangle->w - gBoostImage->w; //回転によるずれの調整差分
			dy = image_reangle->h - gBoostImage->h;
			boost.dst.x = c_center.x + gCharaImage[chara_id]->w/3 - dx/2;
			boost.dst.y = c_center.y - gBoostImage->h/2 - dy/2;
			SDL_BlitSurface(image_reangle, &boost.src, c_window, &boost.dst);
		    }
		}
		if(rtt_flag != ROTATE_NEUTRAL){
		    if(rtt_flag == ROTATE_RIGHT){ //右回転の場合
			image_reangle = rotozoomSurface(gBoostImage, 270, 0.4, 1); //角度の変更
			boost.src.w = image_reangle->w; boost.src.h = image_reangle->h;
			dx = image_reangle->w - gBoostImage->w; //回転によるずれの調整差分
			dy = image_reangle->h - gBoostImage->h;
			for(i = 0; i < 2; i++){ //頭部の噴射炎
			    boost.dst.x = c_center.x - gBoostImage->w/2 - dx/2 + (i+1)*15;
			    boost.dst.y = c_center.y - gCharaImage[chara_id]->h/2 - gBoostImage->h- dy/2 + 10 + i*10;
			    SDL_BlitSurface(image_reangle, &boost.src, c_window, &boost.dst);
			}
			image_reangle = rotozoomSurface(gBoostImage, 90, 0.4, 1);
			boost.src.w = image_reangle->w; boost.src.h = image_reangle->h;
			dx = image_reangle->w - gBoostImage->w;
			dy = image_reangle->h - gBoostImage->h;
			for(i = 0; i < 3; i++){ //後部の噴射炎
			    boost.dst.x = c_center.x - gBoostImage->w/2 - dx/2 - (i+1)*15 + 10;
			    boost.dst.y = c_center.y + gCharaImage[chara_id]->h/2 - dy/2 - 10 - i*10;
			    SDL_BlitSurface(image_reangle, &boost.src, c_window, &boost.dst);
			}
		    }
		    if(rtt_flag == ROTATE_LEFT){ //左回転の場合
			image_reangle = rotozoomSurface(gBoostImage, 90, 0.4, 1); //角度の変更
			boost.src.w = image_reangle->w; boost.src.h = image_reangle->h;
			dx = image_reangle->w - gBoostImage->w; //回転によるずれの調整差分
			dy = image_reangle->h - gBoostImage->h;
			for(i = 0; i < 2; i++){ //頭部
			    boost.dst.x = c_center.x - gBoostImage->w/2 - dx/2 + (i+1)*15;
			    boost.dst.y = c_center.y + gCharaImage[chara_id]->h/2 - dy/2 - 10 - i*10;
			    SDL_BlitSurface(image_reangle, &boost.src, c_window, &boost.dst);
			}
			image_reangle = rotozoomSurface(gBoostImage, 270, 0.4, 1); //角度の変更
			boost.src.w = image_reangle->w; boost.src.h = image_reangle->h;
			dx = image_reangle->w - gBoostImage->w; //回転によるずれの調整差分
			dy = image_reangle->h - gBoostImage->h;
			for(i = 0; i < 3; i++){ //後部
			    boost.dst.x = c_center.x - gBoostImage->w/2 - dx/2 - (i+1)*15 + 10;
			    boost.dst.y = c_center.y - gCharaImage[chara_id]->h/2 - gBoostImage->h- dy/2 + 10 + i*10;
			    SDL_BlitSurface(image_reangle, &boost.src, c_window, &boost.dst);
			}
		    }
		}
		if(image_reangle != NULL){
			SDL_FreeSurface(image_reangle);
		}
}

/*void drawArroundEffect(int use_flag, SDL_Surface c_window){ //キャラの周りのエフェクトの描画

		Rect effect;
		c_center.x = c_window->w/2;
		c_center.y = c_window->h/2;
		effect.src.x = 0; effect.src.y = 0;

		switch(use_flag){
			case ITEM_NOIZING: //ジャミング
				effect.src.w = gNoizing->w;   effect.src.h = gNoizing->h;
				effect.dst.x = c_center.x - gNoizing->w/2;
				effect.dst.y = c_center.y - gNoizing->h/2
				SDL_BlitSurface(gNoizing, &effect.src, c_window, &effect.dst); //描画
				break;
			case ITEM_BARRIER: //バリア
				effect.src.w = gBarrier->w;   effect.src.h = gBarrier->h;
				effect.dst.x = c_center.x - gBarrier->w/2;
				effect.dst.y = c_center.y - gBarrier->h/2;
				SDL_BlitSurface(gBarrier, &effect.src, c_window, &effect.dst); //描画
				break;
			default :
				break;
		}

}
*/
void drawDeadChara(POSITION *charaPos, int chara_id){ //死亡アニメーションの描画
		int interval = 100;
		int animeNum = player[chara_id].deadAnimation / (interval * FPS / 1000);
		if(animeNum >= 16){
			//GAMEOVERの描写
			SDL_Surface *strings;
			SDL_Color red = {204, 0, 0};
			font = TTF_OpenFont(gFontFile, 48);
			strings = TTF_RenderUTF8_Blended(font, "GAME OVER　m9(^Д^)", red);
			SDL_Rect go_dst = {gMainWindow->w/2 - strings->w/2, gMainWindow->h/2 - strings->h/2};
			SDL_Rect go_src = {0, 0, strings->w, strings->h};
			SDL_BlitSurface(strings, &go_src, gMainWindow, &go_dst);
			SDL_FreeSurface(strings);
			return;
		}
		double angle;
		POSITION diffPos;
		POSITION* myPos = &myPlayer->object->pos; //マイポジション
		SDL_Rect src_rect = {0, 0, gBoomImage->w/8, gBoomImage->h/2};
		SDL_Rect dst_rect;
		diffPos.x = charaPos->x - myPos->x - (gBoomImage->w /8 /2);
		diffPos.y = charaPos->y - myPos->y - (gBoomImage->h /2 /2);
		adjustWindowPosition(&dst_rect, &diffPos);

		src_rect.x = (gBoomImage->w / 8) * (animeNum % 8);
		src_rect.y = (gBoomImage->h / 2) * (animeNum / 8);
		SDL_BlitSurface(gBoomImage, &src_rect, gMainWindow, &dst_rect); //描画
}


void drawItem(POSITION *itemPos, int item_id){ //アイテムの描画
		POSITION diffPos;
		POSITION* myPos = &myPlayer->object->pos; //マイポジション
		SDL_Rect dst_rect;
		SDL_Rect src_rect = {0, 0, gItemImage[item_id]->w, gItemImage[item_id]->h};

		diffPos.x = itemPos->x - myPos->x - gItemImage[item_id]->w/2;
		diffPos.y = itemPos->y - myPos->y - gItemImage[item_id]->h/2;
		adjustWindowPosition(&dst_rect, &diffPos); //貼り付け位置を計算
		SDL_BlitSurface(gItemImage[item_id], &src_rect, gMainWindow, &dst_rect); //描画
}

void drawObstacle(POSITION *obsPos){ //障害物の描画

		POSITION diffPos;
		POSITION* myPos = &myPlayer->object->pos; //マイポジション
		SDL_Rect dst_rect;
		SDL_Rect src_rect = {0, 0, ObstacleImage[0]->w, ObstacleImage[0]->h};

		diffPos.x = obsPos->x - myPos->x - (ObstacleImage[0]->w /2);
		diffPos.y = obsPos->y - myPos->y - (ObstacleImage[0]->h /2);
		adjustWindowPosition(&dst_rect, &diffPos); //貼り付け位置を計算
		SDL_BlitSurface(ObstacleImage[0], &src_rect, gMainWindow, &dst_rect); //描画
}

/*
void drawLaser(POSITION *charaPos, int chara_id){ //レーザーの描画

		Rect shadow, entity;
		SDL_Surface *reangle;
		int dir = player[chara_id].dir *HALF_DEGRESS /PI; //キャラの向き(度数)

		//影の描写
		reangle = rotozoomSurface(gLaserImage[1], dir, 1.0, 1.0);
		
}
*/

void drawWarning(void){ //警告の表示
		POSITION* myPos = &myPlayer->object->pos;
		SDL_Surface *image_reangle, *strings;
		double angle, dx, dy;
		char warn_st[100];
		int l, m; //残り時間を表す変数
		int rx = gMiniMapImage->w/2;
		int ry = gMiniMapImage->h/2;

		//警告文の描写
		SDL_Rect warn_src = {0, 0, gWarningImage->w, gWarningImage->h};
		SDL_Rect warn_dst = {100, 100};
		SDL_BlitSurface(gWarningImage, &warn_src, gMainWindow, &warn_dst); //警告マークの表示
		SDL_Color red = {204, 0, 0};
		l = myPlayer->lastTime/1000; //残り時間の整数部分
		m = myPlayer->lastTime%1000; //少数部分
		sprintf(warn_st ,"Back to Area!!!    %d.%d", l,m);
		strings = TTF_RenderUTF8_Blended(font, warn_st, red);
		warn_dst.y += 150;
		warn_src.w = strings->w; warn_src.h = strings->h;
		SDL_BlitSurface(strings, &warn_src, gMainWindow, &warn_dst);

		//中心への矢印
		dx = -myPos->x;
		dy = myPos->y;
		if (dx == 0) {
			angle = dy > 0 ? 90 : -90;
		} else if (dy == 0) {
			angle = dx < 0 ? 180 : 0;
		} else {
			angle = atan2(dy,dx) * HALF_DEGRESS / PI; //角度を求める
		}
		image_reangle = rotozoomSurface(gArrowImage, angle, 1.0, 1); //角度の変更
		SDL_Rect ar_src = {0, 0, image_reangle->w, image_reangle->h};
		SDL_Rect ar_dst;
		double r_angle = angle *PI / HALF_DEGRESS;
		dx = image_reangle->w - gArrowImage->w; //回転によるずれの調整差分
		dy = image_reangle->h - gArrowImage->h;
		ar_dst.x = gMainWindow->w - gMiniMapImage->w/2 + rx*cos(r_angle) - image_reangle->w/2 - dx/2;
		ar_dst.y = gMiniMapImage->h/2 - ry*sin(r_angle) - image_reangle->h/2 - dy/2;
		SDL_BlitSurface(image_reangle, &ar_src, gMainWindow, &ar_dst);

		SDL_FreeSurface(strings);
		SDL_FreeSurface(image_reangle);
		
}


void drawStatus(void){ //ステータスの描画

		//1.ステータスウィンドウに描画
		SDL_Rect src_rect = {0, 0, 0, 0};
		SDL_Rect dst_rect;
		int i;
		int item_id;
		int chara_id;
		for(i=0; i < MAX_CLIENTS; i++){
			if(player[i].object != NULL) {
			    chara_id = player[i].num;	// キャラ番号
			    item_id = player[i].item;	// アイテム番号

			      if(!player[chara_id].alive){ //ゲームオーバーの場合
				   src_rect.x = 0;
				   src_rect.w = gDeadIconImage->w;
				   src_rect.h = gDeadIconImage->h;
				   dst_rect.x = chara_id*gItemBox->w + (gItemBox->w/2 - gDeadIconImage->w)/2;
				   dst_rect.y = (gItemBox->h - gDeadIconImage->h)/2;
				   SDL_BlitSurface(gDeadIconImage, &src_rect, gStatusWindow, &dst_rect);
			      }else{
					//アイコン
			      		src_rect.w = gIconImage[chara_id]->w;    src_rect.h = gIconImage[chara_id]->h;
			      		dst_rect.x = chara_id*gItemBox->w + (gItemBox->w/2 - gIconImage[chara_id]->w)/2;
			      		dst_rect.y = (gItemBox->h - gIconImage[chara_id]->h)/2;
			      		SDL_BlitSurface(gIconImage[chara_id], &src_rect, gStatusWindow, &dst_rect);
			      		//所持アイテム
			     		 if(item_id > ITEM_EMPTY){
						if(chara_id != myID){
							item_id++;
						}
				 		src_rect.w = gItemImage[item_id]->w;
				 		src_rect.h = gItemImage[item_id]->h;
				 		dst_rect.x = chara_id*gItemBox->w + gItemBox->w/2 + (gItemBox->w/2 - gItemImage[item_id]->w)/2 - 5;
				 		dst_rect.y = gItemBox->h/2 - gItemImage[item_id]->h/2;
				 		SDL_BlitSurface(gItemImage[item_id], &src_rect, gStatusWindow, &dst_rect);
					}
				}
			   }
		}

		//2.まとめてメインウィンドウに貼付け
		Rect status;
		status.src.x = 0;   status.src.y = 0;
		status.src.w = gStatusWindow->w;   status.src.h = gStatusWindow->h;
		status.dst.x = 0;   status.dst.y = gMainWindow->h - gStatusWindow->h;
		SDL_BlitSurface(gStatusWindow, &status.src, gMainWindow, &status.dst);
}


void drawMiniMap(POSITION* myPos) { //ミニマップの描画
		//1.gMiniMapの初期化
		SDL_Rect src_rect = {0, 0, gMiniMapImage->w, gMiniMapImage->h};
		SDL_Rect dst_rect = {0, 0};
		SDL_BlitSurface(gMiniMapImage, &src_rect, gMiniMap, &dst_rect);

		//2.オブジェクトの位置をgMiniMapに描写
		int i;
		int rImg = 60;
		int asp = 80; //比率
		int rd = rImg * asp; //ミニマップの半径
		SDL_Rect point[MAX_OBJECT];
		POSITION center;
		center.x = gMiniMap->w / 2;
		center.y = gMiniMap->h / 2;

		for (i = 0; i < MAX_OBJECT; i++) { //オブジェクトの場所の描画
				if (object[i].type == OBJECT_EMPTY)
						continue;
				point[i].w = 5; point[i].h = 5;
				double dx = object[i].pos.x - myPos->x;	
				double dy = object[i].pos.y - myPos->y;
				double range = pow(dx, 2) + pow(dy, 2);
				if (range < pow(rd, 2)) {
						double angle;
						if (dx == 0) {
								angle = dy > 0 ? 90 * PI / HALF_DEGRESS : -90 * PI / HALF_DEGRESS;
						} else if (dy == 0) {
								angle = dx < 0 ? 180 * PI / HALF_DEGRESS : 0;
						}
						angle = atan2(dy,dx); //角度を求める

						point[i].x = center.x + dx / asp;
						point[i].y = center.y + dy / asp;
						printf("point[%d] [%d: %d]\n", i, point[i].x - 67, point[i].y - 67);

						int size = 2;
						switch(object[i].type) {
								case OBJECT_CHARACTER: //キャラクター
										if (((PLAYER*)object[i].typeBuffer)->num == myID) {
												filledCircleColor(gMiniMap, center.x, center.y, 4, 0x00ffffff); //自分
										} else {
												filledCircleColor(gMiniMap, point[i].x, point[i].y, size, 0xffd700); //敵
										}
										break;
								case OBJECT_OBSTACLE: //障害物
										filledCircleColor(gMiniMap, point[i].x, point[i].y, size, 0xff0000ff);
										printf("obstacle\n");
										break;
								case OBJECT_ITEM: //アイテム
										filledCircleColor(gMiniMap, point[i].x, point[i].y, size, 0x0000ffff);
										printf("item\n");
										break;
								default:
										break;
						}
				}
		}

		//3.gMiniMapをメインウィンドウに貼付け
		SDL_Rect map_src = {0, 0, gMiniMap->w, gMiniMap->h};
		SDL_Rect map_dst = {gMainWindow->w - gMiniMap->w, 0};
		SDL_BlitSurface(gMiniMap, &map_src, gMainWindow, &map_dst);
}
