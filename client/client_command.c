#include "../common.h"
#include "client_common.h"
#include "client_func.h"
/*****************************************************************
関数名	: ExecuteCommand
機能	: サーバーから送られてきたコマンドを元に，
		  引き数を受信し，実行する
引数	: char	command		: コマンド
出力	: プログラム終了コマンドがおくられてきた時には0を返す．
		  それ以外は1を返す
*****************************************************************/
int executeCommand(int command)
{
		int	endFlag = 1;
#ifndef NDEBUG
		/*
		printf("#####\n");
		printf("ExecuteCommand()\n");
		*/
#endif
		switch(command){
		case END_COMMAND:
		    endFlag = 0;
                
		    break;
		default:	
		    break;
		}
		return endFlag;
}
/*****************************************************************
関数名	: SendEndCommand
機能	: プログラムの終了を知らせるために，
		  サーバーにデータを送る
引数	: なし
出力	: なし
*****************************************************************/
void sendEndCommand(void)
{
		unsigned char	data[MAX_DATA];
		int	dataSize;

#ifndef NDEBUG
		printf("#####\n");
		printf("sendEndCommand()\n");
#endif

		dataSize = 0;
		/*データの送信*/
		sendData(data, dataSize);
}
