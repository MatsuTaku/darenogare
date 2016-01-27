#ifndef _SERVER_FUNC_H_
#define _SERVER_FUNC_H_

#include"server_common.h"

/* server_main.c */
extern bool gcd(int a, int b);
extern bool lcm(int a, int b);

/* server_net.c */
extern int getClientNum();
extern int setUpServer(int num);
extern void callEndGame();
extern void ending(void);
extern void sendData(int pos,void *data,int dataSize);
extern bool sendRecvManager(void);
extern void sendMapData(int pos);

/* server_command.c */
extern bool executeCommand(int id, entityStateSet* state);

/* server_system.c */
extern void setClientNum(int);
extern void initSystem();
extern void updateBuffer();
extern void updateSystem();
extern void setPlayerState(int id, entityStateSet* state);
extern void sendDeltaBuffer(int id, int latest);


#endif
