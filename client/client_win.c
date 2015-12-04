#include <SDL/SDL.h>
#include <SDL/SDL_image.h>
#include <SDL/SDL_gfxPrimitives.h>
#include <SDL/SDL_ttf.h>
#include <SDL/SDL_rotozoom.h>
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
static char gItemImgFile[ITEM_NUM][20] = {
		"IMG/noizing.png",
		"IMG/Laser.png", 
		"IMG/missile.png",
		"IMG/minimum.png",
		"IMG/barrier.png"
};
static char gCharaImgFile[CT_NUM][20] ={
		"IMG/1Pship.png",
		"IMG/2Pship.png",
		"IMG/3Pship.png",
		"IMG/4Pship.png"
};
static char gIconImgFile[CT_NUM][20] = {
		"IMG/1Picon.png", 
		"IMG/2Picon.png", 
		"IMG/3Picon.png", 
		"IMG/4Picon.png"
};
static char gNameImgFile[CT_NUM][20] = {
		"IMG/1Pname.png",
		"IMG/2Pname.png",
		"IMG/3Pname.png",
		"IMG/4Pname.png"
};


static int hiritu = 100;
static int weitFlag = 0;
static int myID;
static POSITION center;

/* ジョイスティック */
SDL_Joystick* joystick;

/*サーフェース*/
static SDL_Surface *gMainWindow;//メインウィンドウ
static SDL_Surface *gWorldWindow; //各プレイヤーのマップウィンドウ
static SDL_Surface *gStatusWindow; //各プレイヤーのステータスウィンドウ
static SDL_Surface *gWorld;//背景画像のサーフェス
static SDL_Surface *gItemImage[ITEM_NUM];//アイテム
static SDL_Surface *gCharaImage[CT_NUM];//プレイヤー
static SDL_Surface *ObstacleImage[1]; //障害物
static SDL_Surface *gIconImage[CT_NUM];//アイコン
static SDL_Surface *gItemBox; //アイテム欄
static SDL_Surface *gNameImage[CT_NUM]; //キャラクター名

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
		printf("コンパイル %d\n", myPlayer->object->pos.x);

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

		//WorldWindow
		if((gWorldWindow = SDL_CreateRGBSurface(SDL_SWSURFACE, WINDOW_WIDTH, WINDOW_HEIGHT, 32, 0xff000000,0x00ff0000,0x0000ff00,0x000000ff)) == NULL) {
			printf("failed to initialize worldwindow");
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
		myPlayer->object->pos.y = 0;
		myPlayer->object->pos.x = 0;
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
		gWorld = IMG_Load( gMapImgFile ); 
		if ( gWorld == NULL ) { //マップ画像
				printf("not find world image\n");
				return(-1);
		}
		ObstacleImage[0] = IMG_Load( gObstacleImgFile );
		if( ObstacleImage[0] == NULL ){ //障害物画像
				printf("not find obstacle image\n");
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
		for(i = 0; i < CT_NUM; i++){ //キャラクター画像
				gCharaImage[i] = IMG_Load( gCharaImgFile[i] );
				if( gCharaImage[i] == NULL ){
						printf("not find chara%dimage\n", i+1);
						return(-1);
				}
		}
		for(i = 0; i < CT_NUM; i++){ //アイコン画像
				gIconImage[i] = IMG_Load( gIconImgFile[i] );
				if( gIconImage[i] == NULL ){
						printf("not find icon%dimage\n", i+1);
						return(-1);
				}
		}
		for(i = 0; i < CT_NUM; i++){ //アイコン画像
				gNameImage[i] = IMG_Load( gNameImgFile[i] );
				if( gNameImage[i] == NULL ){
						printf("not find name%dPimage\n", i+1);
						return(-1);
				}
		}
}


void clearWindow(void){ //ウィンドウのクリア

	//メインウィンドウ
	SDL_FillRect(gMainWindow, NULL, 0xffffff);

	//ワールドウィンドウ
	SDL_FillRect(gWorldWindow, NULL, 0x000000);
	SDL_SetColorKey(gWorldWindow, SDL_SRCCOLORKEY, SDL_MapRGB(gWorldWindow->format, 0, 0,
0)); // 黒を透過色に指定
	gWorldWindow = SDL_DisplayFormat(gWorldWindow);

	//ステータスウィンドウ
	SDL_Rect src_rect = {0, 0, gItemBox->w, gItemBox->h};
	SDL_Rect dst_rect = {0, 0};
	int i;
	for(i=0; i < CT_NUM; i++){
	  dst_rect.x = i*(gItemBox->w);
	  SDL_BlitSurface(gItemBox, &src_rect, gStatusWindow, &dst_rect);
	  }

}


void drawObject(void) { //オブジェクトの描画
	SDL_Rect src_rect;
	SDL_Rect dst_rect;
	SDL_Surface *image_reangle;
	double angle;
	int i;
	int chara_id, item_id, obstacle_id;
	POSITION objPos;
	src_rect.x = 0;
	src_rect.y = 0;
	POSITION* myPos = &myPlayer->object->pos;
	POSITION diffPos;

	for(i=0; i<MAX_OBJECT; i++){
	//printf("i = %d  type %d\n", i,object[i].type);
	    //if(judgeRange(&object[i].pos, myPos) > 0){ //表示範囲内にあれば
		switch(object[i].type){
		  case OBJECT_CHARACTER: //キャラクター
			chara_id = ((PLAYER*)object[i].typeBuffer)->num; //キャラ番号
			angle = player[chara_id].dir * HALF_DEGRESS / PI; //キャラの向き
			image_reangle = rotozoomSurface(gCharaImage[chara_id], angle, 1.0, 1); //角度の変更
			src_rect.x = 0;
			src_rect.y = 0;
			src_rect.w = image_reangle->w;
			src_rect.h = image_reangle->h;
			int dx = image_reangle->w - src_rect.w; //回転によるずれの調整差分
			int dy = image_reangle->h - src_rect.h;
			diffPos.x = object[i].pos.x - myPos->x - (gCharaImage[chara_id]->w /2) - dx/2;		
			diffPos.y = object[i].pos.y - myPos->y - (gCharaImage[chara_id]->h /2) - dy/2;		
			adjustWindowPosition(&dst_rect, &diffPos);
			//printf("draw chara\n");
			SDL_BlitSurface(image_reangle, &src_rect, gWorldWindow, &dst_rect);
			break;

		  case OBJECT_ITEM: //アイテム
		  	item_id = object[i].id;
			src_rect.w = gItemImage[item_id]->w;
			src_rect.h = gItemImage[item_id]->h;
			diffPos.x = object[i].pos.x - gItemImage[item_id]->w/2;
			diffPos.y = object[i].pos.y - gItemImage[item_id]->h/2;
			adjustWindowPosition(&dst_rect, &diffPos);
			printf("draw item\n");
			SDL_BlitSurface(gItemImage[item_id], &src_rect, gWorldWindow, &dst_rect);
			break;
			
		  case OBJECT_OBSTACLE: //障害物
			src_rect.w = ObstacleImage[0]->w;
			src_rect.h = ObstacleImage[0]->h;
			diffPos.x = object[i].pos.x - (ObstacleImage[0]->w /2);
			diffPos.y = object[i].pos.y - (ObstacleImage[0]->h /2);
			adjustWindowPosition(&dst_rect, &diffPos);
			//printf("draw obstacle\n");
			dst_rect.x = 200;
			dst_rect.y = 200;
			SDL_BlitSurface(ObstacleImage[0], &src_rect, gWorldWindow, &dst_rect);
			break;

		  case OBJECT_EMPTY: //なし
			break;
		//  }
	    }
	}
	if(image_reangle != NULL){
		SDL_FreeSurface(image_reangle);
	}
}


static void adjustWindowPosition(SDL_Rect* windowPos, POSITION* pos) {
		int diffWidth = WINDOW_WIDTH / 2;
		int diffHeight = WINDOW_HEIGHT / 2;
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


void combineWindow(POSITION* myPos){ //各プレイヤーの画面の作成
		
		int asp = 100;
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

		//マップを組み合わせる
		Rect world;
		world.src.x = 0;
		world.src.y = 0;
		world.src.w = WINDOW_WIDTH;
		world.src.h = WINDOW_HEIGHT;
		world.dst.x = 0;
		world.dst.y = 0;
		SDL_BlitSurface(gWorldWindow, &world.src, gMainWindow, &world.dst);

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
				&& objPos->x < (myPos->x + range_h)){ //縦
				return 1;
			}
		}
		return -1;
}
	
			



		



