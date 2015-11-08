/*****************************************************************
ファイル名:	clientver_common.h
機能		: サーバーで使用する定数の宣言を行う
*****************************************************************/

#ifndef _CLIENT_COMMON_H_
#define _CLIENT_COMMON_H_

#include"../common.h"

/* client_net.c */
extern int clientID;

#define ALL_CLIENTS	-1   /* 全クライアントにデータを送る時に使用する */

#endif
