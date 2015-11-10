#include "../common.h"
#include "client_common.h"
#include "client_func.h"
#include <SDL/SDL.h>

int networkEvent(void* data);
int drawThread(void* data);

int main(int argc,char *argv[])
{
		int		num;
		int clientID;
		int		endFlag=1;
		char	localHostName[]="localhost";
		char	*serverName;

	SDL_Thread* thread1;
	SDL_Thread* thread2;

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
	if(initWindows(clientID,num)==-1){
		fprintf(stderr,"setup failed : InitWindows\n");
		return -1;
	}

	thread1 = SDL_CreateThread(networkEvent, (void *)NULL);
	thread2 = SDL_CreateThread(drawThread, (void *)NULL);
	if (thread1 == NULL || thread2 == NULL) {
			printf("\nSDL_CreateThread failed: %s\n", SDL_GetError());
	}

    while(endFlag){
		windowEvent(num);
		endFlag = sendRecvManager();
    };

	destroyWindow();
	closeSoc();

    return 0;
}

int networkEvent(void* data) {
		int endFlag = 1;
		while(endFlag) {
				endFlag = sendRecvManager();
		}
}

int drawThread(void* data) {//キャラ移動などを反映
		int endFlag = 1;
		while(endFlag) {//1なら継続→client_winへ
				endFlag = drawWindow();
		}
}
