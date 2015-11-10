#ifndef _SERVER_FUNC_H_
#define _SERVER_FUNC_H_

#include"server_common.h"

/* server_net.c */
extern int setUpServer(int num);
extern void ending(void);
extern void sendData(int pos,void *data,int dataSize);
extern int sendRecvManager(void);

/* server_command.c */
extern int executeCommand(int command, int pos);

/* server_system.c */
extern void physicsManager(void);

#endif
