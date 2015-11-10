#include"server_common.h"
#include"server_func.h"

static int numCommand;


int executeCommand(int command, int pos) //コマンド処理
{
/*
引数1:コマンド
引数2:送信元
*/
	int	endFlag = 1; //エンドフラグ（0で終了、非0で継続）

	assert(0<=pos && pos<MAX_CLIENTS); //送信元が正しいか確認

	int i;
#ifndef NDEBUG
	printf("#####\n");
	printf("ExecuteCommand()\n");
#endif


		switch(command){
				case END_COMMAND:
						endFlag = 0;
						sendData(ALL_CLIENTS, END_COMMAND, sizeof(int));
						break;
				default:
						fprintf(stderr,"0x%02x is not command!\n", command);
						break;
		}
	return endFlag;
}
