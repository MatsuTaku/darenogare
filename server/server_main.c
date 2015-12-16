#include<SDL/SDL.h>
#include"server_common.h"

static Uint32 SignalHandler(Uint32 interval, void *param);

int main(int argc,char *argv[]) {
		int	num;
		bool endFlag = false;

		if(argc != 2){
				fprintf(stderr,"Usage: number of clients\n");
				exit(-1);
		}
		if((num = atoi(argv[1])) < 0 ||  num > MAX_CLIENTS){
				fprintf(stderr,"clients limit = %d \n",MAX_CLIENTS);
				exit(-1);
		}

		if(SDL_Init(SDL_INIT_TIMER) < 0) {
				printf("failed to initialize SDL.\n");
				exit(-1);
		}

		if (initSystem(num) < 0) {
				printf("failed to initSystem!\n");
				exit(-1);
		}

		if(setUpServer(num) == -1){
				fprintf(stderr,"Cannot setup server\n");
				exit(-1);
		}

		/* ネットワークループ */
		int ms = 1000;
		int a = ms, b = FPS, tmp;
		int r = a % b;
		while(r != 0) {
				a = b;
				b = r;
				r = a % b;
		}
		double gcd = ms * FPS / b;
		Uint32 loopInterval = ms / b;
		int timeRate = FPS / b;
		Uint32 startTime, endTime, toTime;
		while(!endFlag) {
				startTime = SDL_GetTicks() * timeRate;
				toTime = startTime + loopInterval;
				endFlag = sendRecvManager();
				endTime = SDL_GetTicks() * timeRate;
				if (endTime < toTime) {
						SDL_Delay((toTime - endTime) / timeRate);
				}
		}

		ending();
		exit(0);
		return 0;
}
