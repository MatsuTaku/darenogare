/*******************************************************
ファイル名	: UI.c
機能		: ユーザーインターフェース処理
********************************************************/ 
#include <SDL/SDL.h>
#include <SDL/SDL_image.h>
#include <SDL/SDL_ttf.h>
#include <SDL/SDL_gfxPrimitives.h>
#include "common.h"
#include <SDL/SDL_mixer.h>



/* フォント関連 */
static char gFontFileE[] = "/usr/share/fonts/truetype/droid/DroidSerif-Regular.ttf";
static char gFontFileJ[] = "/usr/share/fonts/opentype/ipafont-mincho/ipamp.ttf";
static char *gMsgStrings[ MSG_NUM ] = { "Game Over", "Win!",};
static char *gHelpMsgStr = "Help!";
static TTF_Font *gTTF;

/* サーフェース */
static SDL_Surface *gMainWindow;//メインウィンドウのサーフェース
static SDL_Surface *gTheWorld;//マップ（背景画像）のサーフェース
static SDL_Surface *gMapImage;
static SDL_Surface *gCharaImage[ CT_NUM ];
static SDL_Surface *gMessages[ MSG_NUM ];
static SDL_Surface *gHelpMsg;



/* 画像転送用 */
typedef struct {
    SDL_Rect src;
    SDL_Rect dst;
} Rect;

/*************************************************************
関数名:InitWindow()
機能:ウィンドウ及び背景画像などを作成する.

*************************************************************/
int InitWindow(void)
{

    /* メインのウインドウ(ゲーム画面)の作成 */
    gMainWindow = SDL_SetVideoMode (1024,768, 32, SDL_HWSURFACE|SDL_DOUBLEBUF);
    if(gMainWindow == NULL) {
    printf("Failed of making window\n");
	exit(-1);
    }


 /* マップ画面(フィールドバッファ)の作成 */
    gTheWorld = SDL_SetVideoMode (1024,768, 32, SDL_HWSURFACE|SDL_DOUBLEBUF);
    if(gTheWorld == NULL) {
       printf("Failed map window\n");
	exit(-1);
    }
     

     /* フォントからメッセージ作成 */
    /* 初期化 */
    if(TTF_Init() < 0){
       printf("Failedttf1\n");
	exit(-1);
    }
    /* フォントを開く */
    gTTF = TTF_OpenFont( gFontFileE, 96 );
    if( gTTF == NULL ){ printf("Failedttf2\n");
	exit(-1);
}
        /* メッセージ作成 */
    SDL_Color cols[ MSG_NUM ]  = { {0,0,0}, {255,255,255} };
    int i;
    for(i=0; i<MSG_NUM; i++){
        gMessages[i] = TTF_RenderText_Blended(gTTF, gMsgStrings[i], cols[i]);
        if( gMessages[i] == NULL ){printf("Failedmsg\n");
	exit(-1);
    }
}
    /* フォントを閉じる */
    TTF_CloseFont(gTTF);
    /* フォントを開く */
    gTTF = TTF_OpenFont( gFontFileE, 16 );
    if( gTTF == NULL ){ printf("Failedttf3\n");
	exit(-1);
    }
    /* メッセージ作成 */
    SDL_Color hlpcol = {255,0,0};
    gHelpMsg = TTF_RenderText_Blended(gTTF, gHelpMsgStr, hlpcol);
    if( gHelpMsg == NULL ){ printf("Failedmsg2\n");
	exit(-1);
    }
    /* フォントを閉じる */
    TTF_CloseFont(gTTF);
    /* フォントを開く */
    gTTF = TTF_OpenFont( gFontFileJ, 26 );//フォント（訂正箇所）
    if( gTTF == NULL ) {
	printf("Failedttf4\n");
	exit(-1);
    }
    /* キーリピート無効を設定 */
    SDL_EnableKeyRepeat(0,0);

    return 0;// InitWindowsの終了
}




/*********************************************************
 メインウインドウに対するイベント処理
 
    返値 bool　で判断
 
    継続: 1..true
    終了: 0..false
**********************************************************/
int WindowEvent()
{

    SDL_Event event;     
    if(SDL_PollEvent(&event)){
        switch(event.type){
           case SDL_QUIT:
           	return 0;
	      
           default:            
               return 1;
        }
     } 
 return 1;
}
    

/************************************************************
 <<描画更新用の関数>>

 呼び出されるとウィンドウ上の描画を更新する.


*************************************************************/

int DrawWindow(void)
{
    int ret = 0;
    char title[ 64 ];
    SDL_Surface *mesF, *mesB;
    SDL_Color colF = {255,255,255};
    SDL_Color colB = {0,0,0};
    Rect rect = {{0}};

    sprintf(title,"%d",gStatus.DownPlayer);
    
    mesF = TTF_RenderUTF8_Blended(gTTF, title, colF);
    mesB = TTF_RenderUTF8_Blended(gTTF, title, colB);
    if(mesB){
        rect.src.w = mesB->w;
        rect.src.h = mesB->h;
        rect.dst.x = 1024 - rect.src.w - 16 + 1;
        rect.dst.y = 16 + 1;
        ret += SDL_BlitSurface(mesB, &(rect.src), gTheWorld, &(rect.dst));
        rect.dst.y -= 2;
        ret += SDL_BlitSurface(mesB, &(rect.src), gTheWorld, &(rect.dst));
        SDL_FreeSurface(mesB);
    }
    if(mesF){
        rect.src.w = mesF->w;
        rect.src.h = mesF->h;
        rect.dst.x = 1024 - rect.src.w - 16;
        rect.dst.y = 16;
        ret += SDL_BlitSurface(mesF, &(rect.src), gTheWorld, &(rect.dst));
        rect.dst.x--;
        ret += SDL_BlitSurface(mesF, &(rect.src), gTheWorld, &(rect.dst));
        SDL_FreeSurface(mesF);
    }
 
     return 0;
}




/******************************************************
 end of UI.c
 ******************************************************/
