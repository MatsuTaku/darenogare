/*****************************************************************
¥Õ¥¡¥€¥ëÌŸ:	clientver_common.h
µ¡Çœ		: ¥µ¡Œ¥Ð¡Œ€Ç»ÈÍÑ€¹€ëÄê¿ô€ÎÀëžÀ€ò¹Ô€Š
*****************************************************************/

#ifndef _CLIENT_COMMON_H_
#define _CLIENT_COMMON_H_

#include"../common.h"

/* client_net.c */
extern int clientID;
/* client_win.c */
extern PLAYER player; //プレイヤー(自分)のステータス

#define ALL_CLIENTS	-1   /* ÁŽ¥¯¥é¥€¥¢¥ó¥È€Ë¥Ç¡Œ¥¿€òÁ÷€ë»þ€Ë»ÈÍÑ€¹€ë */

#endif
