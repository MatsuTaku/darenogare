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
bool executeCommand(entityStateGet* data) {
#ifndef NDEBUG
		/*
		   printf("#####\n");
		   printf("ExecuteCommand()\n");
		 */
#endif
		 if (data->endFlag) {
				 return true;
				 printf("Get end command.\n");
		 }
		 reflectDelta(data);

		return false;
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
#ifndef NDEBUG
		printf("#####\n");
		printf("sendEndCommand()\n");
#endif
		entityStateSet data;
		data.endFlag = true;
		data.latestFrame = 0;

		/*データの送信*/
		sendData(&data, sizeof(entityStateSet));
}
