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
		bool	endFlag = false;
		bool 	endNet = false;
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
		networkThread = SDL_CreateThread(networkEvent, &endNet);
		if (networkThread == NULL) {
				printf("\nSDL_CreateThread failed: %s\n", SDL_GetError());
		}


		/* メインループ */
		// 1000とFPSの最小公倍数を基準に分数で計算
		int ms = MIRI_SECOND;
		int src = gcd(ms, FPS);
		Uint32 loopInterval = ms / src;
		int timeRate = FPS / src;
		Uint32 startTime, endTime, toTime;
		while((endFlag = windowEvent()) == false && !endNet){
				startTime = SDL_GetTicks() * timeRate;
				toTime = startTime + loopInterval;
				timerEvent(++frame);
				endTime = SDL_GetTicks() * timeRate;
				if (endTime < toTime) {
						SDL_Delay((toTime - endTime) / timeRate);
				}
#ifndef NDEBUG
				printf("FPS: %d\n", endTime > toTime ? (int)(lcm(ms, FPS) / (endTime - startTime)) : FPS);
#endif
		};

		if (endFlag) {
				sendEndCommand();
				SDL_KillThread(networkThread);
		} else if (endNet) {
				SDL_WaitThread(networkThread, NULL);
		}

		destroyWindow();
		destroySystem();
		closeSoc();
		return 0;
}


static int networkEvent(void* data) {
		bool *endFlag = (bool *)data;
		// 1000とCPSの最小公倍数を基準に分数で計算
		int ms = MIRI_SECOND;
		int src = gcd(ms, CPS);
		Uint32 loopInterval = ms / src;
		int timeRate = CPS / src;
		Uint32 startTime, endTime, toTime;
		while(!*endFlag) {
				startTime = SDL_GetTicks() * timeRate;
				toTime = startTime + loopInterval;
				*endFlag = sendRecvManager();
				endTime = SDL_GetTicks() * timeRate;
				if (endTime < toTime) {
						SDL_Delay((toTime - endTime) / timeRate);
				}
#ifndef NDEBUG
				printf("CPS: %d\n", endTime > toTime ? (int)(lcm(ms, CPS) / (endTime - startTime)) : CPS);
#endif
		};
		return 0;
}


static Uint32 timerEvent(Uint32 frame) {
		int s, e, w;
		s = SDL_GetTicks();
		updateEvent();
		e = SDL_GetTicks();
		drawWindow();
		w = SDL_GetTicks();
	//	printf("time system: %d,	window: %d\n", e - s, w - e);
}


int gcd(int a, int b) {
		if (!a || !b)	return 0;
		int r;
		while ((r = a % b) != 0) {
				a = b;
				b = r;
		}
		return b;
}


int lcm (int a, int b) {
		if (!a || !b)	return 0;
		return (a * b / gcd(a, b));
}
