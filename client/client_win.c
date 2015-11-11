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

int drawWindow()//ここを主に編集
{
    int endFlag = 1;
                

    gMapImage = IMG_Load( gMapImgFile ); //マップデータ読み込み
    if( gMapImage == NULL ){
        printf("Failedreadmapimg\n");//読み込めない時のエラー表示
	exit(-1);
    }

        SDL_Surface* image;
	SDL_Rect rect, scr_rect;    

        image = SDL_LoadBMP("Field.bmp");
    
        rect.x = 0;
	rect.y = 0;
	rect.w = image->w;
	rect.h = image->h;
     
        scr_rect.x = 0;
	scr_rect.y = 0;
        SDL_BlitSurface(image, &rect, gMainWindow, &scr_rect);	
        SDL_Flip(gMainWindow);//描画更新

        return endFlag; //endflagは1で返す(継続)
}

int ItemGet()//触れたアイテムをゲットする
{
 int num;
 if(player.pos.x == .item.pos.x || player.pos.y == OBJECT.item.pos.y){
     num = ITEM_NUM;
 }
 return num;
}

void ItemUse()//アイテムの使用
{
  int num = 0;
  num = ItemGet();
  player.item = num;

  if(player.item != 0){ /*アイテムごとに決められた処理を行う*/
     switch(num){
            case 1: break;
            case 2: break;
            case 3: break;
            case 4: break;
     }
         player.item = 0;/*使用後消去*/ 
  }    

}


void Boost(int accele){ //機体の速度変更
	player.ver.vx += ( accele * cos(player.dir) ); //x方向
	player.ver.vy += ( accele * sin(player.dir) ); //y方向
}

void changeDir(int degree){ //方向転換
//引数:アナログスティックを倒した方向
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
			   changeDir(event.jaxis.axis); //方向転換
			   break;

			case SDL_JOYBUTTONDOWN: //ボタンが押された時
			   switch(event.jbutton.button){ //ボタン毎の処理
			      case 2: //ジェット噴射
			         Boost(1); //速度上昇
			         break;
			      case 4: //ジェット逆噴射
			         Boost(-1); //速度下降（逆方向にブースト）
			         break;
			   }
			   break;


			case SDL_JOYBUTTONUP: //ボタンが離された時
			   if(event.jbutton.button == 6){ //アイテム使用
			      ItemUse();
			}
			   //徐々に減速する
			   if(player.ver.vx > 0){
			      Boost(-1)
			   }
			   if(player.ver.vx < 0){
			      Boost(1)
			   }
			   break;
		}

}








