/*****************************************************************
ファイル名	: server_common.h
機能		: サーバーで使用する定数の宣言を行う
*****************************************************************/

#ifndef _SERVER_COMMON_H_
#define _SERVER_COMMON_H_

#include"../common.h"

/* server_net.c */
extern int gClientNum;

#define ALL_CLIENTS	-1   /* 全クライアントにデータを送る時に使用する */

#endif
