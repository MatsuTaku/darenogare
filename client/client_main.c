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
		int		endFlag = 1;
		char	localHostName[]="localhost";
		char	*serverName;

		int		frame = -1;

		SDL_Thread* networkThread;
		SDL_TimerID drawTimer;

		if (argc == 1) {
				serverName = localHostName;
		} else if (argc == 2) {
				serverName = argv[1];
		} else {
				fprintf(stderr, "Usage: %s, Cannot find a Server Name.\n", argv[0]);
				return -1;
		}

		if (setUpClient(serverName,&clientID,&num) == -1) {
				fprintf(stderr,"setup failed : SetUpClient\n");
				return -1;
		}

		if (initGameSystem(clientID, num) == -1) {
				fprintf(stderr, "initalize failed: initGameSystem\n");
				return -1;
		}

		if (initWindows(clientID,num) == -1) {
				fprintf(stderr,"setup failed : InitWindows\n");
				return -1;
		}


		/* ネットワーク処理スレッド作成 */
		networkThread = SDL_CreateThread(networkEvent, &endFlag);
		if (networkThread == NULL) {
				printf("\nSDL_CreateThread failed: %s\n", SDL_GetError());
		}


		/* メインループ */
		// 1000とFPSの最小公倍数を基準に分数で計算
		int ms = 1000;
		int a = ms, b = FPS, tmp;
		int r = a % b;
		while(r != 0) {
				a = b;
				b = r;
				r = a % b;
		}
		double gcd = ms * FPS / b;
		printf("gcd = %f\n", gcd);
		Uint32 loopInterval = ms / b;
		int timeRate = FPS / b;
		Uint32 startTime, endTime, toTime;
		while((endFlag = windowEvent()) != 0){
				startTime = SDL_GetTicks() * timeRate;
				toTime = startTime * timeRate + loopInterval;
				timerEvent(++frame);
				endTime = SDL_GetTicks() * timeRate;
				if (endTime < toTime) {
						SDL_Delay((toTime - endTime) / timeRate);
				}
#ifndef NDEBUG
				printf("FPS: %d\n", (int)(gcd / (endTime - startTime)));
#endif
		};
	
		// SDL_WaitThread(networkThread, NULL);
		SDL_KillThread(networkThread);

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
		int s, e, w;
		s = SDL_GetTicks();
		updateEvent();
		e = SDL_GetTicks();
		drawWindow();
		w = SDL_GetTicks();
		printf("time system: %d,	window: %d\n", e - s, w - e);
}
