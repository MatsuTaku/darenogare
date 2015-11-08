#ifndef _COMMON_H_
#define _COMMON_H_

#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include<sys/types.h>
#include<assert.h>
#include<math.h>

#define PORT			(u_short)51771	

#define MAX_CLIENTS		2				
#define MAX_NAME_SIZE	10 				

#define MAX_DATA		200				

#define MAX_RESULT		64

#define END_COMMAND		'E'		  		
#define RESULT_COMMAND	'L'
#define ROCK_COMMAND	'R'
#define SCISSORS_COMMAND	'S'
#define PAPER_COMMAND	'P'
#define DEFAULT_COMMAND	'D'

#define EVEN	-1
/*
 *  ファイル名	: system.h
 *  機能	: 共通変数，外部関数の定義
 */  
#ifndef _SYSTEM_H_
#define _SYSTEM_H_

/*UI用に追記*/
#include <SDL/SDL.h>

/* マップサイズ */
enum {
    MAP_Width    = 13,
    MAP_Height   = 10,
    MAP_Center_X =  6,
    MAP_Center_Y =  6,
    MAP_ChipSize = 60
};

/* メッセージ */
enum {
    MSG_GameOver = 0,
    MSG_Win      = 1,
    MSG_NUM      = 2            /* メッセージの数 */
};


/* マップの座標 */
typedef struct {
    int x;
    int y;
} Pos;

/* キャラクタータイプ */
typedef enum {
    CT_Player = 0,
    CT_Enemy  = 1,
    CT_NUM    = 2              /* タイプの数 */
} CharaType;

/* キャラクターの状態 */
typedef enum {
    CS_Wait,                    /* 待機 */
    CS_Att,                     /*撃てる状態*/
    CS_SHOT,                    /* 撃つ */ 
    CS_Down                    /*負ける*/
} CharaStts;

/* ゲームの状態 */
typedef struct {
  double Time;            /*ゲーム開始からの時間経過*/
  int DownPlayer;          /* 倒した人数 */
}GameStts;


/* キャラクターの情報 */
typedef struct {
    Pos       pos;
    CharaStts stts;
    int       target;           /* 追跡・逃走相手，いないときは-1 */
    SDLKey    dir;              /* キャラクターの移動方向 */
    int       anipat;           /* アニメーションパターン */
    int       anipatnum;        /* パターン数 */
} CharaInfo;

/* 変数 */
extern CharaInfo gChara[ CT_NUM ];
extern GameStts gStatus;


/* 関数 */
/* gunman.c */
extern int RandomTime(int k);

/* system.c */
extern int  InitSystem(void);
extern void DestroySystem(void);
extern int startflag(void);
extern int enemymotion(void);
extern int  Judge(void);
extern int Randomtime(void);
extern int jtime;
extern int N;

/* window_ui.c */
extern int  InitWindows(void);
extern void DestroyWindow(void);
extern int WindowEvent();
extern int  BlitWindow(void);
extern void GameStart(int i);
extern void bloody(int i);
extern void Win(void);
extern void GameOver(void);
extern void MapFlip(void);
extern  void ShotSound(void);
extern  void WinSound(void);
extern  void LoseSound(void);
extern  void BGM(void);
extern void Faint(void);
extern void Faint2(void);
extern void Menu(void);
extern void Say1(void);
extern void Say2(void);
extern void Say3(void);



#endif
/* end of system.h */
typedef struct {
		char command;
		char commands[MAX_CLIENTS];
		int winner;
}	RESULT;


#endif
