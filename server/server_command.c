#include"server_common.h"
#include"server_func.h"

static RESULT results[MAX_RESULT];
static RESULT* nowResult;
static int numResult = -1;
static int numCommand;

int executeCommand(char command, int pos) //コマンド処理
{
/*
引数1:コマンド
引数2:送信元
*/
		int			endFlag = 1;

		assert(0<=pos && pos<MAX_CLIENTS); //送信元が正しいか確認

		int i;
#ifndef NDEBUG
		printf("#####\n");
		printf("ExecuteCommand()\n");
#endif

		switch(command){
				case END_COMMAND:
						endFlag = 0;
						nowResult->command = command;
						sendData(ALL_CLIENTS, nowResult, sizeof(RESULT));
						break;
				default:
						fprintf(stderr,"0x%02x is not command!\n", command);
						break;
		}
		return endFlag;
}
