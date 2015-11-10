/********************************************
ファイル名： spsb.c
機能　　　： メインルーチン 
*********************************************/
#include <stdio.h>
#include <SDL/SDL.h>
#include "common.h"
#include <time.h>
//変数
int gWaittime;
GameStts  gStatus;

//関数
extern  void TimerEvent(int time);
extern  void  BGM(void);
extern int WindowEvent();
extern int DrawWindow();
extern int InitWindow();

/********************************
システム処理
*********************************/

int InitSystem(void)
{
  /* 乱数初期化 */
    srand(time(NULL));
      return 0;
}


/***********************************************
タイマーイベント関数

***********************************************/
void TimerEvent(int time)
{  
    if(time %1 == 0){   
          InitSystem(); 
             DrawWindow();       
    }    
}
/*********************************************
main関数

*********************************************/
int main(int argc,char *argv[])
{


    /* 初期化処理 */
    /* SDL */
    if( SDL_Init( SDL_INIT_VIDEO |SDL_INIT_JOYSTICK ) < 0){
        printf("Failed\n");
        exit(-1);	
    }
    /* ゲームシステム */
    if( InitSystem() < 0 ){
          printf("Failed\n");
	    exit(-1);
    }
    /* ウインドウ */
    if( InitWindow() < 0 ){
           printf("Failed\n");
	    exit(-1);
     }
   /*  SDL_Joystick *joystick;
       if(SDL_NumJoysticks() > 0){
	joystick = SDL_JoystickOpen(0);
        }*/

    /* タイマー */
    Uint32 interval = SDL_GetTicks() + 10;

   /* ゲーム内時刻：約0.01秒で1カウント */
    int time = 0;
   
    /* メインループ */
    int loopFlag = 0;
    Uint32 now;
 
       loopFlag = 1;
    
    while(loopFlag){
        /* ウインドウ(SDL)イベント */
           loopFlag = WindowEvent();
        /* タイマー検出 */
               now = SDL_GetTicks();
                  if( now >= interval ){
                     TimerEvent( ++time);
                       /* 0.1秒ごとにタイマー処理するため，0.1秒後を設定 */
                       interval = now + 10; 
                  }

    };


    SDL_Quit();

  


	
return 0;


}
/* end of spsb.c */
