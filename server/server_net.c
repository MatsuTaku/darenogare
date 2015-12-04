#include "server_common.h"
#include "server_func.h"
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>

typedef struct{
	int		fd;
}CLIENT;

static CLIENT	gClients[MAX_CLIENTS];
int	gClientNum;				

static fd_set	gMask;		
static int	gWidth;

static int multiAccept(int request_soc,int num);
static void enter(int pos, int fd);
static void setMask(int maxfd);
static void sendAllName(void);
static int recvData(int pos,void *data,int dataSize);

int setUpServer(int num)//サーバの立ち上げ
{
    struct sockaddr_in	server; //ソケットの宣言
    int			request_soc; //受付用ソケット
    int                 maxfd;
    int			val = 1; //基本設定のオプション
 
    assert(0<num && num<=MAX_CLIENTS);

    gClientNum = num;
    bzero((char*)&server,sizeof(server));
    server.sin_family = AF_INET;
    server.sin_port = htons(PORT);
    server.sin_addr.s_addr = htonl(INADDR_ANY);

/********* 1.(受付)ソケットの生成と基本設定 *********/
    if((request_soc = socket(AF_INET,SOCK_STREAM,0)) < 0){ //ソケットの生成（ネットワークアドレスの種類、ソケットの種類、プロトコル(0は自動生成)）
		fprintf(stderr,"Socket allocation failed\n");
		return -1;
    }
    setsockopt(request_soc,SOL_SOCKET,SO_REUSEADDR,&val,sizeof(val)); //基本設定(ソケットディスクリプタ、ソケットレベル、設定する項目、設定のバイト数)


    if(bind(request_soc,(struct sockaddr*)&server,sizeof(server))==-1){ //bind:ソケットに設定を結びつける（ソケット、設定、設定のサイズ）
		fprintf(stderr,"Cannot bind\n");
		close(request_soc);
		return -1;
    }
    fprintf(stderr,"Successfully bind!\n");
   
/*********** 2.接続ソケットの準備 ************/
    if(listen(request_soc, gClientNum) == -1){ //受付ソケットの準備（受付用ソケット、接続受付最大人数）
		fprintf(stderr,"Cannot listen\n");
		close(request_soc);
		return -1;
    }
    fprintf(stderr,"Listen OK\n");


/************ 3.接続ソケットの生成と接続の確立 *************/
    maxfd = multiAccept(request_soc, gClientNum); //接続の確立
    close(request_soc); //受付用ソケットを閉じる
    if(maxfd == -1)return -1;

    sendAllName();

    setMask(maxfd);

    return 0;
}

int sendRecvManager(void) //ネットワークメインモジュール
{

    fd_set	readOK;
    int		i, command;
    int		endFlag = 1;
    entityStateSet data;
    memset (&data, 0, sizeof(entityStateSet));

    readOK = gMask;
    if(select(gWidth,&readOK,NULL,NULL,NULL) < 0){ //読み込み可能なFDを探す
        return endFlag; //なければ終了
    }

    for(i=0;i<gClientNum;i++){ //全てのクライアントに対して
		if(FD_ISSET(gClients[i].fd,&readOK)){ //読み込み可能なFDがあれば
			     recvData(i, &data, sizeof(entityStateSet)); //受信
			     endFlag = executeCommand(i, &data); //コマンド処理
	    	                 if(endFlag == 0)
                                    break; //終了コマンドが押されたら脱ループ
		}
    }
    return endFlag;
}

void sendData(int pos,void *data,int dataSize)
{
/*
引数1:送り先
引数2:送信内容
引数3:データサイズ
*/
    int	i;
   
    //assert:エラー診断
    assert(0<=pos && pos<gClientNum || pos==ALL_CLIENTS); //送信先が正しいか確認
    assert(data!=NULL); //データが存在するか確認
    assert(0<dataSize); //データサイズの確認

    if(pos == ALL_CLIENTS){ //全員に送信する
		for(i=0;i<gClientNum;i++){
			write(gClients[i].fd,data,dataSize);
		}
    }
    else{ //個人に送信する
		write(gClients[pos].fd,data,dataSize);
    }
}

void terminate_server(void) {//サーバ閉鎖をする関数
  int i;
  for (i = 0; i < gClientNum; i++) {
    close(gClients[i].fd);
  }
  fprintf(stderr, "All connections are closed.\n");
  exit(0);
}


void ending(void)
{
    int i;

     printf("... Connection closed\n");
       for(i=0;i < gClientNum;i++)
            close(gClients[i].fd); //クライアントを終了させる   
    
         
}  


/*****
static
*****/
static int multiAccept(int request_soc,int num) //接続の確率
{
/*
引数1:受付用ソケット
引数2:クライアント数
*/
    int	i,j;
    int	fd;
    
    for(i=0;i<num;i++){
		if((fd = accept(request_soc,NULL,NULL)) == -1){ //接続
			fprintf(stderr,"Accept error\n");
			for(j=i-1;j>=0;j--)close(gClients[j].fd);
			return -1;
		}
		enter(i,fd);
    }
    return fd;
}

static void enter(int pos, int fd)
{
/*
引数1:送信元
引数2:FD
*/
	gClients[pos].fd = fd; //ファイルディスクリプタの格納
}

static void setMask(int maxfd)
{
    int	i;

    gWidth = maxfd+1;

    FD_ZERO(&gMask);
    for(i=0;i<gClientNum;i++)FD_SET(gClients[i].fd,&gMask);
}

static void sendAllName(void)
{
  int	i,j,tmp1,tmp2;

    tmp2 = htonl(gClientNum); //クライアント数のバイトオーダー
    for(i=0;i<gClientNum;i++){
		tmp1 = htonl(i);
		sendData(i,&tmp1,sizeof(int));
		sendData(i,&tmp2,sizeof(int)); //クライアント番号を振り分け
	}
}

static int recvData(int pos,void *data,int dataSize)
{
    int n;
    
    assert(0<=pos && pos<gClientNum);
    assert(data!=NULL);
    assert(0<dataSize);

    n = read(gClients[pos].fd,data,dataSize);
    
    return n;
}
