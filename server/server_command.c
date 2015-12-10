#include"server_common.h"
#include"server_func.h"
#include"../common.h"

static int numCommand;


/**
 *  機能:コマンド処理
 *  引数1:送信元
 *  引数2:entityState
 */


int executeCommand(int pos, entityStateSet* state)
{               /*引数チェック*/
		assert(0<=pos && pos<MAX_CLIENTS); //送信元が正しいか確認
		assert(state != NULL);	// データが入っているか
		
                int endFlag = 1; //エンドフラグ（0で終了、非0で継続）
		entityStateSet* setEntity[MAX_CLIENTS];

		setEntity[pos] = state;

#ifndef NDEBUG
	    /*
		   printf("#####\n");
		   printf("ExecuteCommand()\n");
                 */ 
#endif

		if (state->endFlag == 0) {
                    endFlag = 0;
		}
        return endFlag;
}
