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
static char gMapImgFile[] = "Field.png";
static char ObstacleImgFile[] = "obstacle.png";
static char Item1ImgFile[] = "Thunder.png";
static char Item2ImgFile[] = "";
static char Item3ImgFile[] = "";
static char Item4ImgFile[] = "";
static char Item5ImgFile[] = "";
static char gChara1ImgFile[] = "";
static char gChara2ImgFile[] = "";
static char gChara3ImgFile[] = "";
static char gChara4ImgFile[] = "";
static char gIcon1ImgFile[] = "";
static char gIcon2ImgFile[] = "";
static char gIcon3ImgFile[] = "";
static char gIcon4ImgFile[] = "";

static int weitFlag = 0;
static int myID;

/*サーフェース*/
static SDL_Surface *gMainWindow;//メインウィンドウ
static SDL_Surface *gWorld;//背景画像
static SDL_Surface *gItemImage[ITEM_NUM];//アイテム
static SDL_Surface *gCharaImage[CT_NUM];//プレイヤー
static SDL_Surface *ObstacleImage[1]; //障害物
static SDL_Surface *gIconImage[CT_NUM];//アイコン

/*関数*/
static void drawObject();
static void drawStatus();
static int initImage();


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

		SDL_Rect src_rect = {0, 0, gWorld->w, gWorld->h};
		SDL_Rect dst_rect = {0, 0};
		SDL_BlitSurface(gWorld, &src_rect, gMainWindow, &dst_rect);

		drawObject(); //オブジェクトの描画
		drawStatus(); //ステータスの描画

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

		if (SDL_PollEvent(&event)) {	// イベント所得
				switch(event.type) { 
						case SDL_JOYAXISMOTION: //方向キーorアナログスティック
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
		if( gIconImage == NULL){
			printf("not find icon image\n");
			return(-1);
		}
		gIconImage[1] = IMG_Load( gIcon2ImgFile );
		if( gIconImage == NULL){
			printf("not find icon image\n");
			return(-1);
		}
		gIconImage[2] = IMG_Load( gIcon3ImgFile );
		if( gIconImage == NULL){
			printf("not find icon image\n");
			return(-1);
		}
		gIconImage[3] = IMG_Load( gIcon4ImgFile );
		if( gIconImage == NULL){
			printf("not find icon image\n");
			return(-1);
		}
}



void drawObject(void){ //オブジェクトの描画
	SDL_Rect src_rect;
	SDL_Rect dst_rect;
	int i;
	src_rect.x = 0;
	src_rect.y = 0;

	for(i=0; i<MAX_OBJECT; i++){
		switch(allObject[i].type){

		  case OBJECT_CHARACTER: //キャラクター
			int id = allObject[i].id;
			src_rect.w = gCharaImage[id]->w;
			src_rect.h = gCharaImage[id]->h;
			dst_rect.x = allObject.pos.x - (gCharaImage[id]->w /2);
			dst_rect.y = allObject.pos.y - (gCharaImage[id]->h /2);
			SDL_BlitSurface(gCharaImage[id], &src_rect, gMainWindow, &dst_rect);
			break;

		  case OBJECT_ITEM: //アイテム
		  	int id = allObject[i].id;
			src_rect.w = gItemImage[id]->w;
			src_rect.h = gItemImage[id]->h;
			dst_rect.x = allObject.pos.x - (gItemImage[id]->w /2);
			dst_rect.y = allObject.pos.y - (gItemImage[id]->h /2);
			SDL_BlitSurface(gItemImage[id], &src_rect, gMainWindow, &dst_rect);
			break;
			
		  case OBJECT_OBSTACLE: //障害物
			int id = allObject[i].id;
			src_rect.w = ObstacleImage[id]->w;
			src_rect.h = ObstacleImage[id]->h;
			dst_rect.x = allObject.pos.x - (ObstacleImage[id]->w /2);
			dst_rect.y = allObject.pos.y - (ObstacleImage[id]->h /2);
			SDL_BlitSurface(ObstacleImage[id], &src_rect, gMainWindow, &dst_rect);
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

		int i;
		int item_id;
		int chara_id;

		for(i=0; i<4; i++){
		    chara_id = allPlayer.id;
		    item_id = allPlayer.item;
		    //アイコン
		    src_rect.w = gIconImage[chara_id]->w;
		    src_rect.h = gIconImage[chara_id]->h;
		    dst_rect.x = chara_id*150;
		    dst_rect.y = 600;
		    SDL_BlitSurface(gIconImage[chara_id], &src_rect, gMainWindow, &dst_rect);
		    //所持アイテム
		    if(item_id == 0) break;
		    src_rect.w = gItemImage[item_id]->w;
		    src_rect.h = gItemImage[item_id]->h;
		    dst_rect.x = (chara_id*150) + 50;
		    SDL_BlitSurface(gItemImage[item_id], &src_rect, gMainWindow, &dst_rect);
		}
}








