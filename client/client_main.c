#include "../common.h"
#include "client_common.h"
#include "client_func.h"
#include <SDL/SDL.h>

static int networkEvent(void* data);
static Uint32 drawEvent(Uint32 interval, void* param);
static Uint32 timerEvent(Uint32 frame);


int main(int argc,char *argv[])
{
		int		num;
		int 	clientID;
		int		endFlag=1;
		char	localHostName[]="localhost";
		char	*serverName;

		int		frame;

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

		SDL_Joystick *joystick; //ジョイスティック用構造体
		if(SDL_NumJoysticks() > 0){ //ジョイスティックが接続されたら
				joystick = SDL_JoystickOpen(0); //ジョイスティックをオープン
		} else{ //接続に失敗
				fprintf(stderr,"failed to connect joystick\n");
				return -1;
		}


		Uint32 loopInterval = 1000 / FPS;
		Uint32 nowTime;
		Uint32 toTime = SDL_GetTicks() + loopInterval;
		while(endFlag){
				endFlag = windowEvent(num);

				nowTime = SDL_GetTicks();
				if (nowTime >= toTime) {
						timerEvent(frame++);
						toTime = nowTime + loopInterval;
				}
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


static Uint32 timerEvent(Uint32 frame) {
		updateEvent();
		drawWindow();
}
