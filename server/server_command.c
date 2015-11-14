#include"server_common.h"
#include"server_func.h"
#include"../common.h"

static int numCommand;


int executeCommand(int pos) //コマンド処理
{
/*
引数1:コマンド
引数2:送信元
*/

	int	endFlag = 1; //エンドフラグ（0で終了、非0で継続）
	assert(0<=pos && pos<MAX_CLIENTS); //送信元が正しいか確認

	int i;
#ifndef NDEBUG
	/*
	printf("#####\n");
	printf("ExecuteCommand()\n");
	*/
#endif
	sendData(ALL_CLIENTS, &MAP, sizeof(mapData)); //各クライアントに送信

	return endFlag;
}
