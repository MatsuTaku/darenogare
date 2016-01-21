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
static char gMapImgFile[] = "IMG/WallL.gif"; //マップ
static char gRockImgFile[] = "IMG/obstacle.png"; //隕石
static char gItemBoxImgFile[] = "IMG/Itembox.png"; //アイテムボックス
static char gBoostImgFile[] = "IMG/boost.png"; //噴射炎
static char gWarningImgFile[] = "IMG/warning.png"; //警告マーク
static char gBoomImgFile[] = "IMG/boom.png"; //爆発
static char gDeadIconImgFile[] = "IMG/Deadicon.png"; //死亡時のキャラ
static char gMiniMapImgFile[] = "IMG/minimap.png"; //ミニマップ
static char gTargetImgFile[] = "IMG/target.png"; //目的地
static char gMissileImgFile[] = "IMG/missileEff.png"; //ミサイル
static char gNoizingImgFile[] = "IMG/noizingEff.png"; //妨害電波
static char gBarrierImgFile[] = "IMG/barrierEff.png"; //バリア
static char gLaserImgFile[2][20] = { //レーザー
		"IMG/LaserEff.png",
		"IMG/LaserEff2.png"
};
static char gItemImgFile[ITEM_NUM][20] = { //アイテム
		"IMG/noizing.png",
		"IMG/Laser.png", 
		"IMG/missile.png",
		"IMG/minimum.png",
		"IMG/barrier.png"
};
static char gCharaImgFile[MAX_CLIENTS][20] ={ //キャラ
		"IMG/1Pship.png",
		"IMG/2Pship.png",
		"IMG/3Pship.png",
		"IMG/4Pship.png"
};
static char gIconImgFile[MAX_CLIENTS][20] = { //アイコン
		"IMG/1Picon.png", 
		"IMG/2Picon.png", 
		"IMG/3Picon.png", 
		"IMG/4Picon.png"
};

/* フォントファイル */
static char gFontFile[] = "/usr/share/fonts/opentype/ipafont-mincho/ipamp.ttf";

/* 変数 */
static int now, interval; //アニメーション用の時間変数
static int pn_anm; //お仕置きアニメーション
static int pn_flag; //お仕置きフラグ
static int weitFlag = 0;
static int myID; //プレイヤー番号

/* フォント */
TTF_Font* wFont;
TTF_Font* gFont;

/* ジョイスティック */
SDL_Joystick* joystick;

/*サーフェース*/
static SDL_Surface *gMainWindow;//メインウィンドウ
static SDL_Surface *gBackGround; //背景
static SDL_Surface *gStatusWindow; //各プレイヤーのステータスウィンドウ
static SDL_Surface *gIcon[MAX_CLIENTS];//アイコン
static SDL_Surface *gDeadIcon; //死亡時のアイコン
static SDL_Surface *gItemBox; //アイテム欄
static SDL_Surface *gCharaImage[MAX_CLIENTS];//プレイヤー
static SDL_Surface *gBoostImage; //ブースト
static SDL_Surface *gItemImage[ITEM_NUM];//アイテム
static SDL_Surface *gRockImage; //障害物
static SDL_Surface *gMissileImage; //ミサイル
static SDL_Surface *gNoizingImage; //妨害電波（ジャミング）
static SDL_Surface *gBarrierImage; //バリア
static SDL_Surface *gLaserImage[2]; //レーザー
static SDL_Surface *gWarningImage; //警告
static SDL_Surface *gBoomImage; //爆発
static SDL_Surface *gMiniMap; //ミニマップ用ウィンドウ
static SDL_Surface *gMiniMapImage; //ミニマップの画像
static SDL_Surface *gTargetImage; //中心位置への方向

/*関数*/
static int initImage();
static SDL_Surface* loadImage(char* imageName);
static void drawObject();
static void drawChara(POSITION *charaPos, int chara_id);
static void drawArroundEffect(MODE mode, SDL_Surface *c_window);
static void drawBoost(int chara_id, SDL_Surface *c_window);
static void drawForecast(int id, POSITION *charaPos);
static void drawDeadChara(POSITION *charaPos, int chara_id);
static void drawItem(POSITION *itemPos, int item_id);
static void drawObstacle(POSITION *obsPos, int obs_id, double obs_dir);
static void drawRock_Missile(SDL_Surface* ObsImage, double angle, POSITION *obsPos);
//static void drawLaser(POSITION *charaPos, int chara_id);
static void drawStatus();
static void drawWarning();
static void drawPunishment();
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

/*ウィンドウ生成*/
int initWindows(int clientID, int num) {
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
		wFont = TTF_OpenFont(gFontFile, 24);
		gFont = TTF_OpenFont(gFontFile, 48);
		//各グローバル変数の初期化
		interval = SDL_GetTicks() + 100;
		pn_flag = 0;
		pn_anm = 0;

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

/*ゲーム画面の描画*/
int drawWindow() {

		int endFlag = 1;
		clearWindow(); //ウィンドウのクリア
		now = SDL_GetTicks(); //現在の時刻を取得
		if(now >= interval){ //0.1秒経ったら
			if(pn_flag != 0){
				pn_anm++;
			}
			interval = now + 100; //次の開始時間を0.1秒後に設定
		}
		drawObject(); //オブジェクトの描画
		//drawLaser(); //レーザーの描画
		drawStatus(); //ステータスの描画
		if(myPlayer->alive){ //生存状態
			drawMiniMap(&myPlayer->object->pos); //ミニマップの描画
			if(myPlayer->warn == WARN_OUT_AREA){
				drawWarning(); //警告文の表示
			}
		}else{ //死亡状態
			if(myPlayer->lastTime <= 0 && pn_flag == 0 && pn_anm == 0 && myPlayer->warn == WARN_OUT_AREA){
				pn_flag = now%2 +1; //お仕置きフラグをオンに
				pn_anm++;
			}
			if(pn_flag != 0){
				drawPunishment(); //お仕置きアニメーション
			}
		}
		SDL_Flip(gMainWindow);//描画更新
		return endFlag; //endflagは1で返す(継続)
}

/*サーフェスの解放*/
void destroyWindow(void) {
		SDL_FreeSurface(gMainWindow);
		SDL_FreeSurface(gStatusWindow);
		SDL_FreeSurface(gMiniMap);
		SDL_FreeSurface(gBackGround);
		SDL_FreeSurface(gRockImage);
		SDL_FreeSurface(gItemBox);
		SDL_FreeSurface(gMiniMapImage);
		SDL_FreeSurface(gTargetImage);
		SDL_FreeSurface(gBoomImage);
		SDL_FreeSurface(gDeadIcon);
		SDL_FreeSurface(gWarningImage);
		SDL_FreeSurface(gMissileImage);
		SDL_FreeSurface(gNoizingImage);
		SDL_FreeSurface(gBarrierImage);
		int i;
		for (i = 0; i < ITEM_NUM; i++){
				SDL_FreeSurface (gItemImage[i]);
		}
		for (i = 0; i < MAX_CLIENTS; i++){
				SDL_FreeSurface (gCharaImage[i]);
				SDL_FreeSurface (gIcon[i]);
		}
		for (i = 0; i < 2; i++)
				SDL_FreeSurface (gLaserImage[i]);

		if (joystick)
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
								// printf("press buton: %d\n", event.jbutton.button);
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
										case BUTTON_TRIANGLE:	//アイテム使用
												useItem();
												break;
								}
								break;
						case SDL_JOYBUTTONUP: //ボタンが離された時
								switch (event.jbutton.button) {
										case BUTTON_CIRCLE:
										case BUTTON_CROSS:
												inertialNavigation();
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

static int initImage(void) { //画像の読み込み
		int i, endFlag = 0;

		if ((gBackGround = loadImage(gMapImgFile)) == NULL) endFlag = -1;	// 背景画像
		if ((gRockImage = loadImage(gRockImgFile)) == NULL)	endFlag = -1;	// 障害物
		if ((gBoostImage = loadImage(gBoostImgFile)) == NULL)	endFlag = -1;	// ブースト
		if ((gItemBox = loadImage(gItemBoxImgFile)) == NULL)	endFlag = -1;	// アイテムボックス
		if ((gDeadIcon = loadImage(gDeadIconImgFile)) == NULL)	endFlag = -1;	// 死亡時のアイコン
		if ((gWarningImage = loadImage(gWarningImgFile)) == NULL)	endFlag = -1;	// 警告文
		if ((gBoomImage = loadImage(gBoomImgFile)) == NULL)	endFlag = -1;	// 爆発アニメーション
		if ((gMiniMapImage = loadImage(gMiniMapImgFile)) == NULL)	endFlag = -1;	// ミニマップ
		if ((gTargetImage = loadImage(gTargetImgFile)) == NULL)	endFlag = -1;	// 目的地
		if ((gMissileImage = loadImage(gMissileImgFile)) == NULL)	endFlag = -1;	// ミサイル
		if ((gNoizingImage = loadImage(gNoizingImgFile)) == NULL)	endFlag = -1;	// ジャミング
		if ((gBarrierImage = loadImage(gBarrierImgFile)) == NULL)	endFlag = -1;	// バリア
		for (i = 0; i < MAX_CLIENTS; i++) //キャラクター画像
				if ((gCharaImage[i] = loadImage(gCharaImgFile[i])) == NULL)	endFlag = -1;
		for (i = 0; i < ITEM_NUM; i++) //アイテム画像
				if ((gItemImage[i] = loadImage(gItemImgFile[i])) == NULL)	endFlag = -1;
		for (i = 0; i < 2; i++) //レーザー画像
				if ((gLaserImage[i] = loadImage(gLaserImgFile[i])) == NULL)	endFlag = -1;
		for (i = 0; i < MAX_CLIENTS; i++) //アイコン画像
				if ((gIcon[i] = loadImage(gIconImgFile[i])) == NULL)	endFlag = -1;

		return endFlag;
}


static SDL_Surface* loadImage(char* imageName) {
		SDL_Surface* image;
		if (image = IMG_Load(imageName))
				printf("Load [%s]\n", imageName);
		else
				printf("Not found [%s]\n", imageName);
		return image;
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

/*描画用の座標に変換*/
static void adjustWindowPosition(SDL_Rect* windowPos, POSITION* pos) {
		int diffWidth = WINDOW_WIDTH / 2; //y軸の中心
		int diffHeight = WINDOW_HEIGHT / 2; //x軸の中心
		windowPos->x = diffWidth + pos->x;
		windowPos->y = diffHeight + pos->y;
}

/*オブジェクトの描画*/
void drawObject(void) {

	int i;
	int id;
	double dir;
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
				}else{
					drawChara(&object[i].pos, id); //キャラクターの描画
				}
			}
			break;
		  case OBJECT_ITEM: //アイテム
		  	id = ((ITEM *)object[i].typeBuffer)->num; //アイテム番号
			drawItem(&object[i].pos, id); //アイテムの描画
			break;
		  case OBJECT_OBSTACLE: //障害物
			id = ((OBSTACLE*)object[i].typeBuffer)->num; //障害物番号
			dir = ((OBSTACLE*)object[i].typeBuffer)->angle * HALF_DEGRESS / PI; //角度（度数）
			drawObstacle(&object[i].pos, id, dir); //障害物の描画
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


/*範囲内のオブジェクトを検知*/
int judgeRange(POSITION *objPos, POSITION *myPos){
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

/*キャラクターの描画*/
void drawChara(POSITION *charaPos, int chara_id){
		static SDL_Surface *c_window, *reImage; //回転後のサーフェス
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
		//1.特定のアイテム使用時のエフェクトをc_windowに描画
		drawArroundEffect(player[chara_id].mode, c_window);
		//2.噴射炎をc_windowに描画
		drawBoost(chara_id, c_window);
		//3.キャラをc_windowに描画
		SDL_Rect src_rect = {0, 0, gCharaImage[chara_id]->w, gCharaImage[chara_id]->h};
		SDL_Rect dst_rect = {c_center.x - gCharaImage[chara_id]->w/2, c_center.y - gCharaImage[chara_id]->h/2};
		SDL_BlitSurface(gCharaImage[chara_id], &src_rect, c_window, &dst_rect);
		//4.c_windowをウインドウに描画
		angle = player[chara_id].dir * HALF_DEGRESS / PI; //キャラの向き
		if(chara_id != myID){
			angle -= 90;
		}
		double size;
		if(player[chara_id].mode == MODE_MINIMUM){ //ミニマム状態ならサイズを小さく
			size = 0.5;
		}else{
			size = 1.0;
		}
		reImage = rotozoomSurface(c_window, angle, size, 1); //角度の変更
		SDL_Rect c_rect = {0, 0, reImage->w, reImage->h};
		dx = reImage->w - c_window->w; //回転によるずれの調整差分
		dy = reImage->h - c_window->h;
		diffPos.x = charaPos->x - myPos->x - (c_window->w /2) - dx/2;
		diffPos.y = charaPos->y - myPos->y - (c_window->h /2) - dy/2;
		adjustWindowPosition(&dst_rect, &diffPos);
		SDL_BlitSurface(reImage, &c_rect, gMainWindow, &dst_rect); //描画
		/*レーザの予測線の描画*/
		if(player[chara_id].action == ACTION_CD_LASER){
			drawForecast(chara_id, charaPos);
		}

		SDL_FreeSurface(reImage);
		SDL_FreeSurface(c_window);

}

/*キャラ周りのエフェクトの描画*/
void drawArroundEffect(MODE mode, SDL_Surface *c_window){
		Rect effect;
		POSITION c_center;
		c_center.x = c_window->w/2;
		c_center.y = c_window->h/2;
		effect.src.x = 0; effect.src.y = 0;

		switch(mode){
			case 125: //ジャミング
				effect.src.w = gNoizingImage->w;   effect.src.h = gNoizingImage->h;
				effect.dst.x = c_center.x + gCharaImage[myID]->w/2;
				effect.dst.y = c_center.y - gNoizingImage->h/2;
				SDL_BlitSurface(gNoizingImage, &effect.src, c_window, &effect.dst); //描画
				break;
			case MODE_BARRIER: //バリア
				effect.src.w = gBarrierImage->w;   effect.src.h = gBarrierImage->h;
				effect.dst.x = c_center.x - gBarrierImage->w/2;
				effect.dst.y = c_center.y - gBarrierImage->h/2;
				SDL_BlitSurface(gBarrierImage, &effect.src, c_window, &effect.dst); //描画
				break;
			default :
				break;
		}

}



/*噴射炎を描画*/
void drawBoost(int chara_id, SDL_Surface *c_window){
		SDL_Surface* reImage;
		POSITION c_center;
		Rect boost;
		int dx, dy, i;
		int bst_flag = player[chara_id].boost; //噴射フラグ
		int rtt_flag = player[chara_id].rotate; //回転フラグ

		c_center.x = c_window->w/2;
		c_center.y = c_window->h/2;
		boost.src.x = 0; boost.src.y = 0; 
		/*前進・後退*/
		if(bst_flag != BOOST_NEUTRAL){
			if(bst_flag == BOOST_GO){ //前噴射の場合
				for(i = 0; i < 2; i++){
					reImage = rotozoomSurface(gBoostImage, i*-2, 0.5, 1); //縮小
					boost.src.w = reImage->w; boost.src.h = reImage->h;
					dx = reImage->w - gBoostImage->w; //調整差分
					dy = reImage->h - gBoostImage->h;
					boost.dst.x = c_center.x - gCharaImage[chara_id]->w*1.0 -4 - dx;
					boost.dst.y = (c_center.y - reImage->h/2 - 1) - i*26 - dy;
					SDL_BlitSurface(reImage, &boost.src, c_window, &boost.dst);
				}
			    }
		 	if(bst_flag == BOOST_BACK){ //後噴射の場合
				reImage = rotozoomSurface(gBoostImage, HALF_DEGRESS, 1.0, 1); //角度の変更
				boost.src.w = reImage->w; boost.src.h = reImage->h;
				dx = reImage->w - gBoostImage->w; //回転によるずれの調整差分
				dy = reImage->h - gBoostImage->h;
				boost.dst.x = c_center.x + gCharaImage[chara_id]->w/3 - dx/2;
				boost.dst.y = c_center.y - gBoostImage->h/2 - dy/2;
				SDL_BlitSurface(reImage, &boost.src, c_window, &boost.dst);
		 	   }
			SDL_FreeSurface(reImage);
		}
		/*回転*/
		if(rtt_flag != ROTATE_NEUTRAL){
			if(rtt_flag == ROTATE_RIGHT){ //右回転の場合
				reImage = rotozoomSurface(gBoostImage, 270, 0.4, 1); //角度の変更
				boost.src.w = reImage->w; boost.src.h = reImage->h;
				dx = reImage->w - gBoostImage->w; //回転によるずれの調整差分
				dy = reImage->h - gBoostImage->h;
				for(i = 0; i < 2; i++){ //頭部の噴射炎
				    boost.dst.x = c_center.x - gBoostImage->w/2 - dx/2 + (i+1)*15;
				    boost.dst.y = c_center.y - gCharaImage[chara_id]->h/2 - gBoostImage->h- dy/2 + 10 + i*10;
				    SDL_BlitSurface(reImage, &boost.src, c_window, &boost.dst);
				}
				reImage = rotozoomSurface(gBoostImage, 90, 0.4, 1);
				boost.src.w = reImage->w; boost.src.h = reImage->h;
				dx = reImage->w - gBoostImage->w;
				dy = reImage->h - gBoostImage->h;
				for(i = 0; i < 3; i++){ //後部の噴射炎
				    boost.dst.x = c_center.x - gBoostImage->w/2 - dx/2 - (i+1)*15 + 10;
				    boost.dst.y = c_center.y + gCharaImage[chara_id]->h/2 - dy/2 - 10 - i*10;
				    SDL_BlitSurface(reImage, &boost.src, c_window, &boost.dst);
				}
			    }
			if(rtt_flag == ROTATE_LEFT){ //左回転の場合
				reImage = rotozoomSurface(gBoostImage, 90, 0.4, 1); //角度の変更
				boost.src.w = reImage->w; boost.src.h = reImage->h;
				dx = reImage->w - gBoostImage->w; //回転によるずれの調整差分
				dy = reImage->h - gBoostImage->h;
				for(i = 0; i < 2; i++){ //頭部
				    boost.dst.x = c_center.x - gBoostImage->w/2 - dx/2 + (i+1)*15;
				    boost.dst.y = c_center.y + gCharaImage[chara_id]->h/2 - dy/2 - 10 - i*10;
				    SDL_BlitSurface(reImage, &boost.src, c_window, &boost.dst);
				}
				reImage = rotozoomSurface(gBoostImage, 270, 0.4, 1); //角度の変更
				boost.src.w = reImage->w; boost.src.h = reImage->h;
				dx = reImage->w - gBoostImage->w; //回転によるずれの調整差分
				dy = reImage->h - gBoostImage->h;
				for(i = 0; i < 3; i++){ //後部
				    boost.dst.x = c_center.x - gBoostImage->w/2 - dx/2 - (i+1)*15 + 10;
				    boost.dst.y = c_center.y - gCharaImage[chara_id]->h/2 - gBoostImage->h- dy/2 + 10 + i*10;
				    SDL_BlitSurface(reImage, &boost.src, c_window, &boost.dst);
				}
			}
			SDL_FreeSurface(reImage);
		}
}

/*レーザの予測線の描画*/
void drawForecast(int id, POSITION* charaPos){

		SDL_Rect src_rect, dst_rect;
		POSITION  fcPos,diffPos;
		POSITION* myPos = &myPlayer->object->pos; //マイポジション
		SDL_Surface* reImage;
		int angle = player[id].dir * HALF_DEGRESS / PI; //角度（度数）
		double r_angle = player[id].dir; //角度（ラジアン）

		fcPos.x = charaPos->x + (gLaserImage[0]->w *cos(player[id].dir));
		fcPos.y = charaPos->y + (gLaserImage[0]->h *sin(player[id].dir));
		reImage = rotozoomSurface(gLaserImage[0], angle, 0.7, 1.0); //画像の回転
		int dx = reImage->w - gLaserImage[0]->w; //調整差分
		int dy = reImage->h - gLaserImage[0]->h;
		src_rect.x = 0;		src_rect.y = 0;
		src_rect.w = reImage->w;	src_rect.h = reImage->h;
		double rx, ry;
		rx = reImage->w/2*cos(r_angle);
		ry = -reImage->h/2*sin(r_angle);
		diffPos.x = charaPos->x - myPos->x - (gLaserImage[0]->w /2) + rx - dx/2;
		diffPos.y = charaPos->y - myPos->y - (gLaserImage[0]->h /2) + ry - dy/2;
		adjustWindowPosition(&dst_rect, &diffPos);
		SDL_BlitSurface(reImage, &src_rect, gMainWindow, &dst_rect);
		if(reImage != NULL){
			SDL_FreeSurface(reImage);
		}
}


/*死亡キャラの描画*/
void drawDeadChara(POSITION *charaPos, int chara_id){
		if(pn_flag == 1){
			return;
		}
		int interval = 100;
		int animeNum = player[chara_id].deadAnimation / (interval * FPS / 1000);
		if(animeNum >= 16){
			if(!myPlayer->alive){ //"GAME OVER"の描写
				SDL_Surface *strings;
				SDL_Color red = {204, 0, 0};
				strings = TTF_RenderUTF8_Blended(gFont, "GAME OVER", red);
				SDL_Rect go_dst = {gMainWindow->w/2 - strings->w/2, gMainWindow->h/2 - strings->h/2};
				SDL_Rect go_src = {0, 0, strings->w, strings->h};
				SDL_BlitSurface(strings, &go_src, gMainWindow, &go_dst);
				SDL_FreeSurface(strings);
			}
		} else {
				//爆発アニメーション
				POSITION* myPos = &myPlayer->object->pos; //マイポジション
				int imgWidth = gBoomImage->w / 8;
				int imgHeight = gBoomImage->h / 2;
				Rect rect = {
						.src.x = imgWidth * (animeNum % 8),
						.src.y = imgHeight * (animeNum / 8),
						.src.w = imgWidth,
						.src.h = imgHeight,
				};
				POSITION diffPos;
				diffPos.x = charaPos->x - myPos->x - (imgWidth /2);
				diffPos.y = charaPos->y - myPos->y - (imgHeight /2);
				adjustWindowPosition(&rect.dst, &diffPos);

				SDL_BlitSurface(gBoomImage, &rect.src, gMainWindow, &rect.dst); //描画
		}
}


/*アイテムの描画*/
void drawItem(POSITION *itemPos, int item_id){
		POSITION diffPos;
		POSITION* myPos = &myPlayer->object->pos; //マイポジション
		SDL_Rect dst_rect;
		SDL_Rect src_rect = {0, 0, gItemImage[item_id]->w, gItemImage[item_id]->h};

		diffPos.x = itemPos->x - myPos->x - gItemImage[item_id]->w/2;
		diffPos.y = itemPos->y - myPos->y - gItemImage[item_id]->h/2;
		adjustWindowPosition(&dst_rect, &diffPos); //貼り付け位置を計算
		SDL_BlitSurface(gItemImage[item_id], &src_rect, gMainWindow, &dst_rect); //描画
}

/*障害物の描画*/
void drawObstacle(POSITION *obsPos, int obs_id, double obs_dir){
		switch(obs_id){
			case OBS_ROCK: //隕石
				drawRock_Missile(gRockImage, obs_dir, obsPos);
				break;
			case OBS_MISSILE: //ミサイル
				drawRock_Missile(gMissileImage, obs_dir, obsPos);
				break;
			case OBS_LASER: //レーザの描画
				//drawLaser(obsPos, obs_dir);
				break;
		}
}

/*隕石とミサイルの描画*/
void drawRock_Missile(SDL_Surface *ObsImage, double angle, POSITION *obsPos){
		SDL_Surface* reImage;
		POSITION diffPos;
		POSITION* myPos = &myPlayer->object->pos; //マイポジション

		reImage = rotozoomSurface(ObsImage, angle, 1.0, 1); //角度の変更
		SDL_Rect src_rect = {0, 0, reImage->w, reImage->h};
		int dx = reImage->w - ObsImage->w;
		int dy = reImage->h - ObsImage->h;
		diffPos.x = obsPos->x - myPos->x - reImage->w/2 - dx/2;
		diffPos.y = obsPos->y - myPos->y - reImage->h/2 - dy/2;
		SDL_Rect dst_rect;
		adjustWindowPosition(&dst_rect, &diffPos); //貼り付け位置を計算
		SDL_BlitSurface(reImage, &src_rect, gMainWindow, &dst_rect); //描画
		if(reImage != NULL){
			SDL_FreeSurface(reImage);
		}
}

/*レーザの描画*/
/*void drawLaser(SDL_Surface *ObsImage, POSITION *lsPos, int angle){

		Rect shadow, entity;
		SDL_Surface *reImage;
		POSITION diffPos;

		//レーザーの描写
		reImage = rotozoomSurface(gLaserImage[1], dir, 1.0, 1.0);
		dx = reImage->w - gLaserImage[1]->w;	dy = reImage->h - gLaserImage[1]->h;
		int div = 10;
		entity.src.x = gLaserImage[1]->w - gLaserImage[1]->w/div * 経過秒数;	entity.src.y = 0;
		entity.src.w = gLaserImage[1]->w/div * 経過秒数;	  entity.src.h = gLaserImage[1]->h;
		diffPos.x = ポジション - myPos->x - reImage->w/2 - dx;
		diffPos.y = ポジション - myPos->y - reImage->h/2 - dy;
		adjustWindowPosition(&entity.dst, &diffPos);
		SDL_BlitSurface(reImage, &entity.src, gMainWindow, &entity.dst);
		
}*/


/*警告の描画*/
void drawWarning(void){
		POSITION* myPos = &myPlayer->object->pos;
		SDL_Surface *reImage, *strings;
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
		strings = TTF_RenderUTF8_Blended(wFont, warn_st, red);
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
		reImage = rotozoomSurface(gTargetImage, angle, 1.0, 1); //角度の変更
		SDL_Rect ar_src = {0, 0, reImage->w, reImage->h};
		SDL_Rect ar_dst;
		double r_angle = angle *PI / HALF_DEGRESS;
		dx = reImage->w - gTargetImage->w; //回転によるずれの調整差分
		dy = reImage->h - gTargetImage->h;
		ar_dst.x = gMainWindow->w - gMiniMapImage->w/2 + rx*cos(r_angle) - reImage->w/2 - dx/2;
		ar_dst.y = gMiniMapImage->h/2 - ry*sin(r_angle) - reImage->h/2 - dy/2;
		SDL_BlitSurface(reImage, &ar_src, gMainWindow, &ar_dst);

		SDL_FreeSurface(strings);
		SDL_FreeSurface(reImage);
		
}


/*範囲外タブーによる死亡演出*/
void drawPunishment(void){
		SDL_Surface *meteoImage;
		int div;
		if(pn_flag == 1){ //1.外部からのレーザー攻撃
			div = 10;
			Rect laser;
			laser.src.x = gLaserImage[1]->w - gLaserImage[1]->w/div * pn_anm;	laser.src.y = 0;
			laser.src.w = gLaserImage[1]->w/div * pn_anm;	  laser.src.h = gLaserImage[1]->h;
			laser.dst.x = 0;
			laser.dst.y = gMainWindow->h/2 - gLaserImage[1]->h/2;
			SDL_BlitSurface(gLaserImage[1], &laser.src, gMainWindow, &laser.dst);
		}else if(pn_flag == 2){ //2.巨大隕石の襲来
			div = 5;
			Rect meteo;
			meteoImage = rotozoomSurface(gRockImage, 90, 6.0, 1); //画像の巨大化
			meteo.src.x = 0;	meteo.src.y = 0;
			meteo.src.w = meteoImage->w;		meteo.src.h = meteoImage->h;
			int dx = meteoImage->w - gRockImage->w; //画像の調整差分
			int dy = meteoImage->h - gRockImage->h;
			meteo.dst.x = gMainWindow->w - (gMainWindow->w/div * pn_anm) - dx;
			meteo.dst.y = gMainWindow->h/2 - meteoImage->h/2;
			SDL_BlitSurface(meteoImage, &meteo.src, gMainWindow, &meteo.dst);
		}
			

		if(pn_anm >= div){
			pn_flag = 0; //アニメーション終了
		}
}


/*ステータスウィンドウの描画*/
void drawStatus(void){

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
				   src_rect.w = gDeadIcon->w;
				   src_rect.h = gDeadIcon->h;
				   dst_rect.x = chara_id*gItemBox->w + (gItemBox->w/2 - gDeadIcon->w)/2;
				   dst_rect.y = (gItemBox->h - gDeadIcon->h)/2;
				   SDL_BlitSurface(gDeadIcon, &src_rect, gStatusWindow, &dst_rect);
			      }else{
					//アイコン
			      		src_rect.w = gIcon[chara_id]->w;    src_rect.h = gIcon[chara_id]->h;
			      		dst_rect.x = chara_id*gItemBox->w + (gItemBox->w/2 - gIcon[chara_id]->w)/2;
			      		dst_rect.y = (gItemBox->h - gIcon[chara_id]->h)/2;
			      		SDL_BlitSurface(gIcon[chara_id], &src_rect, gStatusWindow, &dst_rect);
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


/*ミニマップの描画*/
void drawMiniMap(POSITION* myPos) {
		//1.gMiniMapの初期化
		SDL_FillRect(gMiniMap, NULL, 0xffffff); //白で塗りつぶし
		SDL_SetColorKey(gMiniMap, SDL_SRCCOLORKEY, SDL_MapRGB(gMiniMap->format, 255, 255, 255)); //白を透過
		gMiniMap = SDL_DisplayFormat(gMiniMap);
		SDL_Rect src_rect = {0, 0, gMiniMapImage->w, gMiniMapImage->h};
		SDL_Rect dst_rect = {0, 0};
		SDL_BlitSurface(gMiniMapImage, &src_rect, gMiniMap, &dst_rect);

		//2.オブジェクトの位置をgMiniMapに描写
		int i;
		int rImg = 60;
		int asp = 40; //比率
		int rd = rImg * asp; //ミニマップの半径
		POSITION center;
		center.x = gMiniMap->w / 2;
		center.y = gMiniMap->h / 2;

		for (i = 0; i < MAX_OBJECT; i++) { //オブジェクトの場所の描画
				if (object[i].type == OBJECT_EMPTY)	continue;
				double dx = object[i].pos.x - myPos->x;	
				double dy = object[i].pos.y - myPos->y;
				double range = pow(dx, 2) + pow(dy, 2);
				if (range < pow(rd, 2)) {
						POSITION point;
						point.x = center.x + dx / asp;
						point.y = center.y + dy / asp;

						int size = 2;
						switch(object[i].type) {
								case OBJECT_CHARACTER: //キャラクター
										if (((PLAYER*)object[i].typeBuffer)->num == myID) {
												filledCircleColor(gMiniMap, center.x, center.y, 4, 0x00ffffff); //自分
										} else {
												filledCircleColor(gMiniMap, point.x, point.y, size, 0xffd700); //敵
										}
										break;
								case OBJECT_OBSTACLE: //障害物
										filledCircleColor(gMiniMap, point.x, point.y, size, 0xff0000ff);
										break;
								case OBJECT_ITEM: //アイテム
										filledCircleColor(gMiniMap, point.x, point.y, size, 0x0000ffff);
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
