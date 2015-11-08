#ifndef _CLIENT_FUNC_H_
#define _CLIENT_FUNC_H_

#include"../common.h"

/* client_net.c */
extern int setUpClient(char* hostName,int *clientID,int *num,char clientName[][MAX_NAME_SIZE]);
extern void closeSoc(void);
extern int recvData(void *data, int dataSize);
extern int recvIntData(int *intData);
extern void sendData(void *data,int dataSize);
extern int sendRecvManager(void);

/* client_win.c */
extern int initWindows(int clientID,int num,char name[][MAX_NAME_SIZE]);
extern void destroyWindow(void);
extern void windowEvent(int num);

/* client_command.c */
extern int executeCommand(int command);
extern void sendEndCommand(void);

#endif
