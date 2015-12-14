#ifndef _CLIENT_FUNC_H_
#define _CLIENT_FUNC_H_

#include"../common.h"

/* client_main.c */

/* client_net.c */
extern int setUpClient(char* hostName,int *clientID,int *num);
extern void closeSoc(void);
extern int recvData(void *data, int dataSize);
extern int recvIntData(int *intData);
extern void sendData(void *data,int dataSize);
extern bool sendRecvManager(void);

/* client_win.c */
extern int initWindows(int clientID,int num);
extern void destroyWindow(void);
extern bool windowEvent();
extern int drawWindow();

/* client_command.c */
extern bool executeCommand(entityStateGet* data, int* latest);
extern void sendEndCommand(void);

/* client_system.c */
extern int initGameSystem(int myId, int playerNum);
extern void updateEvent();
extern void getItem();
extern void useItem();
extern void rotateTo(int, int);
extern void rotateLeft();
extern void rotateRight();
extern void fixRotation();
extern void acceleration();
extern void deceleration();
extern void inertialNavigation();
extern void relectDelta(entityStateGet* data, int* latest);
extern void sendEntity(int latest);

#endif
