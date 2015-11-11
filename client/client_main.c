#include "../common.h"
#include "client_common.h"
#include "client_func.h"
#include <SDL/SDL.h>

static int networkEvent(void* data);
static Uint32 drawEvent(Uint32 interval, void* param);



int main(int argc,char *argv[])
{
		int		num;
		int clientID;
		int		endFlag=1;
		char	localHostName[]="localhost";
		char	*serverName;

		SDL_Thread* networkThread;
		SDL_TimerID drawTimer;

		if(argc == 1){
				serverName = localHostName;
		}
		else if(argc == 2){
				serverName = argv[1];
		}
		else{
				fprintf(stderr, "Usage: %s, Cannot find a Server Name.\n", argv[0]);
				return -1;
		}

		if(setUpClient(serverName,&clientID,&num)==-1){
				fprintf(stderr,"setup failed : SetUpClient\n");
				return -1;
		}

		if (initGameSystem(clientID, num) == -1) {
				fprintf(stderr, "initalize failed: initGameSystem\n");
				return -1;
		}

		if(initWindows(clientID,num)==-1){
				fprintf(stderr,"setup failed : InitWindows\n");
				return -1;
		}

		networkThread = SDL_CreateThread(networkEvent, &endFlag);
		if (networkThread == NULL) {
				printf("\nSDL_CreateThread failed: %s\n", SDL_GetError());
		}

		Uint32 drawInterval = 1000 / 60;	// 60fps
		printf("fps: %3d\n", drawInterval);
		drawTimer = SDL_AddTimer(drawInterval, drawEvent, NULL);

		SDL_Joystick *joystick; //ジョイスティック用構造体
		if(SDL_NumJoysticks() > 0){ //ジョイスティックが接続されたら
				joystick = SDL_JoystickOpen(0); //ジョイスティックをオープン
		} else{ //接続に失敗
				fprintf(stderr,"failed to connect joystick\n");
				return -1;
		}

		while(endFlag){
				windowEvent(num);
		};

		destroyWindow();
		closeSoc();

		return 0;
}

static int networkEvent(void* data) {
		int* endFlag;
		endFlag = (int*)data;
		while(*endFlag) {
				*endFlag = sendRecvManager();
		}
		return 0;
}

static Uint32 drawEvent(Uint32 interval, void* param) {
		drawWindow();
}
