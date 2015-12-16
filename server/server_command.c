#include"server_common.h"
#include"server_func.h"
#include"../common.h"

static int numCommand;

/*
 *  機能:コマンド処理
 *  引数1:送信元
 *  引数2:entityState
 */


int executeCommand(int pos, entityStateSet* state,int command)
{               /*引数チェック*/
		assert(0<=pos && pos<MAX_CLIENTS); //送信元が正しいか確認
		assert(state != NULL);	// データが入っているか
		
                int i = MAX_CLIENTS;
                int endFlag = 1; //エンドフラグ（0で終了、非0で継続）
		entityStateSet* setEntity[MAX_CLIENTS];
		setEntity[pos] = state;


#ifndef NDEBUG
	          /*
		   printf("#####\n");
		   printf("ExecuteCommand()\n");
                 */ 
#endif
             if(state->endFlag == 1){
                 endFlag =0;
             }
        return endFlag;
}


/*****
static
*****/
/*****************************************************************
関数名	: SetIntData2DataBlock
機能	: int 型のデータを送信用データの最後にセットする
引数	: void		*data		: 送信用データ
		  int		intData		: セットするデータ
		  int		*dataSize	: 送信用データの現在のサイズ
出力	: なし
*****************************************************************/
static void SetIntData2DataBlock(void *data,int intData,int *dataSize)
{
    int tmp;

    /* 引き数チェック */
    assert(data!=NULL);
    assert(0<=(*dataSize));

    tmp = htonl(intData);

    /* int 型のデータを送信用データの最後にコピーする */
    memcpy(data + (*dataSize),&tmp,sizeof(int));
    /* データサイズを増やす */
    (*dataSize) += sizeof(int);
}
/*****
static
*****/
/*****************************************************************
関数名	: SetCharData2DataBlock
機能	: char 型のデータを送信用データの最後にセットする
引数	: void		*data		: 送信用データ
		  int		intData		: セットするデータ
		  int		*dataSize	: 送信用データの現在のサイズ
出力	: なし
*****************************************************************/
static void SetCharData2DataBlock(void *data,char charData,int *dataSize)
{
    /* 引き数チェック */
    assert(data!=NULL);
    assert(0<=(*dataSize));

    /* int 型のデータを送信用データの最後にコピーする */
    *(char *)(data + (*dataSize)) = charData;
    /* データサイズを増やす */
    (*dataSize) += sizeof(char);
}
