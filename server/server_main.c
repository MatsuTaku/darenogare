
#include<SDL/SDL.h>
#include"server_common.h"

static Uint32 SignalHandler(Uint32 interval, void *param);

int main(int argc,char *argv[])
{
	int	num;
	int	endFlag = 1;

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

	if(setUpServer(num) == -1){
		fprintf(stderr,"Cannot setup server\n");
		exit(-1);
	}
	
	while(endFlag){
		endFlag = sendRecvManager();
	};

	ending();

	return 0;
}
