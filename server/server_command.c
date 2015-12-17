#include"server_common.h"
#include"server_func.h"
#include"../common.h"


/**
 *  機能:コマンド処理
 *  引数1:送信元
 *  引数2:entityState
 */
bool executeCommand(int id, entityStateSet* state) {               /*引数チェック*/
		assert(0<=id && id<MAX_CLIENTS); //送信元が正しいか確認
		assert(state != NULL);	// データが入っているか
		if (state->endFlag)
				return true;

		setPlayerState(id, state);
		return false;

#ifndef NDEBUG
		/*
		   printf("#####\n");
		   printf("ExecuteCommand()\n");
		 */ 
#endif
}
