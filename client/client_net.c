#include "../common.h"
#include "client_common.h"
#include "client_func.h"
#include <sys/socket.h>
#include <netdb.h>

#define	BUF_SIZE	100


entityStateSet rocket;
mapData MAP;

static int	gSocket;/*ソケット*/
static fd_set	gMask;	/*select()用のマスク*/
static int	gWidth;	/*gMask中のチェックすべきビット数*/	

static void getAllName(int *clientID,int *num);
static void setMask(void);
static int RecvData(void *data,int dataSize);
/*****************************************************************
関数名	: SetUpClient
機能	: サーバーとのコネクションを設立し，
		  ユーザーの名前の送受信を行う
引数	: char	*hostName		: ホスト
		  int	*num			: 全クライアント数
		  char	clientNames[][]		: 全クライアントのユーザー名
出力	: コネクションに失敗した時-1,成功した時0
*****************************************************************/
int setUpClient(char *hostName,int *clientID,int *num)
{
    struct hostent	*servHost;
    struct sockaddr_in	server;
 /* ホスト名からホスト情報を得る */
    if((servHost = gethostbyname(hostName))==NULL){
		fprintf(stderr,"Unknown host\n");
		return -1;
    }

    bzero((char*)&server,sizeof(server));
    server.sin_family = AF_INET;
    server.sin_port = htons(PORT);
    bcopy(servHost->h_addr,(char*)&server.sin_addr,servHost->h_length);

  /* ソケットを作成する */
    if((gSocket = socket(AF_INET,SOCK_STREAM,0)) < 0){
		fprintf(stderr,"socket allocation failed\n");
		return -1;
    }
   /* サーバーと接続する */
    if(connect(gSocket,(struct sockaddr*)&server,sizeof(server)) == -1){
		fprintf(stderr,"cannot connect\n");
		close(gSocket);
		return -1;
    }
    fprintf(stderr,"connected\n");


    printf("Please Wait\n");

    getAllName(clientID,num);

    setMask();
    
    return 0;
}

/*****************************************************************
関数名	: SendRecvManager
機能	: サーバーから送られてきたデータを処理する
引数	: なし
出力	: プログラム終了コマンドが送られてきた時0を返す．
		  それ以外は1を返す
*****************************************************************/
int sendRecvManager(void)
{
    fd_set	readOK;
    int     command;
    int		i;
    int		endFlag = 1;
    struct timeval	timeout;

    timeout.tv_sec = 0;
    timeout.tv_usec = 20;

    readOK = gMask;
  /* サーバーからデータが届いているか調べる */
    select(gWidth,&readOK,NULL,NULL,&timeout);
    if(FD_ISSET(gSocket,&readOK)){
		recvData(&MAP,sizeof(MAP)); /* コマンドを読み込む */
		endFlag = executeCommand(command);
    }
     sendData(&rocket, sizeof(entityStateSet)); //プレイヤーのステータスを送信
    return endFlag;
}
/*****************************************************************
関数名	: RecvIntData
機能	: サーバーからint型のデータを受け取る
引数	: int		*intData	: 受信したデータ
出力	: 受け取ったバイト数
*****************************************************************/
int recvIntData(int *intData)
{
    int n,tmp;
    
    assert(intData!=NULL);

    n = recvData(&tmp,sizeof(int));
    (*intData) = ntohl(tmp);
    
    return n;
}
/*****************************************************************
関数名	: SendData
機能	: サーバーにデータを送る
引数	: void		*data		: 送るデータ
		  int		dataSize	: 送るデータのサイズ
出力	: なし
*****************************************************************/
void sendData(void *data, int dataSize)
{
    assert(data != NULL);
    assert(0 < dataSize);

    write(gSocket,data,dataSize);
}

/*****************************************************************
関数名	: CloseSoc
機能	: サーバーとのコネクションを切断する
引数	: なし
出力	: なし
*****************************************************************/
void closeSoc(void) 
{
    printf("...Connection closed\n");
    close(gSocket);
}

/*****
static
*****/
/*****************************************************************
関数名	: getAllName
機能	: サーバーから全クライアントのユーザー名を受信する
引数	: int		*num			: クライアント数
	  int           *clientID
出力	: なし
*****************************************************************/
static void getAllName(int *clientID,int *num)
{
    int	i;

    recvIntData(clientID);
    recvIntData(num);
}
/*****************************************************************
関数名	: setMask
機能	: select()のためのマスク値を設定する
引数	: なし
出力	: なし
*****************************************************************/
static void setMask(void)
{
    int	i;

    FD_ZERO(&gMask);
    FD_SET(gSocket,&gMask);

    gWidth = gSocket+1;
}
/*****************************************************************
関数名	: RecvData
機能	: サーバーからデータを受け取る
引数	: void		*data		: 受信したデータ
		  int		dataSize	: 受信するデータのサイズ
出力	: 受け取ったバイト数
*****************************************************************/

int recvData(void *data,int dataSize)
{   /* 引き数チェック */
    assert(data != NULL);
    assert(0 < dataSize);

    return read(gSocket,data,dataSize);
}
