#ifndef _CLIENT_FUNC_H_
#define _CLIENT_FUNC_H_

#include "../common.h"
#include "SDL/SDL.h"

/* client_main.c */
extern int gcd(int a, int b);
extern int lcm(int a, int b);

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

extern SDL_Surface *loadImage(char *);

/* client_command.c */
extern bool executeCommand(entityStateGet* data);
extern void sendEndCommand(void);

/* client_system.c */
extern void initGameSystem(int myId, int playerNum);
extern int initSystem();
extern void finalSystem();
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
extern void relectDelta(syncData);
extern void sendEntity();

#endif
