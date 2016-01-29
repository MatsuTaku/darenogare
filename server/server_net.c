#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include "server_common.h"
#include "server_func.h"

typedef struct {
		int	fd;
} CLIENT;

static CLIENT	gClients[MAX_CLIENTS];
static int	gClientNum;				

static fd_set	gMask;		
static int	gWidth;

static int multiAccept(int request_soc, int num);
static void enter(int pos, int fd);
static void setMask(int maxfd);
static void sendAllName(void);
static int recvData(int pos, void *data, int dataSize);

int getClientNum() {
		return gClientNum;
}

/*****************************************************************
  関数名	: SetUpServer
  機能	: クライアントとのコネクションを設立し，
  ユーザーの名前の送受信を行う
  引数	: int		num		  : クライアント数
  出力	: コネクションに失敗した時-1,成功した時0
 *****************************************************************/
int setUpServer(int num) {
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
		//ソケットの生成（ネットワークアドレスの種類、ソケットの種類、プロトコル(0は自動生成)）
		if ((request_soc = socket(AF_INET,SOCK_STREAM,0)) < 0) {
				fprintf(stderr,"Socket allocation failed\n");
				return -1;
		}
		//基本設定(ソケットディスクリプタ、ソケットレベル、設定する項目、設定のバイト数)
		setsockopt(request_soc,SOL_SOCKET,SO_REUSEADDR,&val,sizeof(val));


		//bind:ソケットに設定を結びつける（ソケット、設定、設定のサイズ）
		if (bind(request_soc,(struct sockaddr*)&server,sizeof(server)) == -1) {
				fprintf(stderr, "Cannot bind\n");
				close(request_soc);
				return -1;
		}
		printf("Successfully bind!\n");

		/*********** 2.接続ソケットの準備 ************/
		//受付ソケットの準備（受付用ソケット、接続受付最大人数）
		if (listen(request_soc, gClientNum) == -1) {
				fprintf(stderr,"Cannot listen\n");
				close(request_soc);
				return -1;
		}
		printf("Listen OK\n");


		/************ 3.接続ソケットの生成と接続の確立 *************/
		//接続の確立
		maxfd = multiAccept(request_soc, gClientNum);
		//受付用ソケットを閉じる
		close(request_soc); 
		if (maxfd == -1)	return -1;

		sendAllName();

		setMask(maxfd);

		return 0;
}

bool loadingSync() {
		bool endFlag = false;
		fd_set readOK = gMask;

		if (select(gWidth, &readOK, NULL, NULL, NULL) < 0) {
				return endFlag;
		}

		int i;
		for (i = 0; i < gClientNum; i++) {
				if (FD_ISSET((gClients[i].fd), &readOK)) {
						syncData data;
						recvData(i, &data, sizeof(syncData));
						if (data.common.endFlag)	endFlag = true;
						if (data.type == DATA_PREPARE)
								nowLoadingClient(i);
				}
		}
		if (!endFlag) {
				updateLoading();
		} else {
				callEndGame();
		}

		return endFlag;
}

bool battleSync() {
		bool endFlag = false;
		fd_set readOK = gMask;

		updateBuffer();

		if (select(gWidth, &readOK, NULL, NULL, NULL) < 0) {	// 読み込み可能なFDを探す
				return endFlag;
		}
		
		syncData data[gClientNum];
		int i;
		bool recvId[gClientNum];
		for (i = 0; i < gClientNum; i++) { //全てのクライアントに対して
				recvId[i] = false;
				if (FD_ISSET(gClients[i].fd, &readOK)) { //読み込み可能なFDがあれば
						recvData(i, &data[i], sizeof(syncData)); //受信
						if (data[i].common.endFlag) {
								endFlag = true;
								break;
						}
						if (data[i].type != DATA_ES_SET)
								continue;
#ifndef NDEBUG
						// printf("recvData[from: %d]\n", i);
#endif
						recvId[i] = true;
						setPlayerState(i, &data[i].set);
				}
		}

		updateSystem();

		if (!endFlag) {
				for (i = 0; i < gClientNum; i++) {
						if (recvId[i]) {
								sendDeltaBuffer(i, data[i].set.latestFrame);
						}
				}
		} else {
				callEndGame();
		}

		return endFlag;
}

void callEndGame() {
		syncData end = {
				.common.type = DATA_COMMON,
				.common.endFlag = true,
		};
		sendData(ALL_CLIENTS, &end, sizeof(syncData));
}

/*****************************************************************
  関数名	: SendData
  機能	: クライアントにデータを送る
  引数	: int	   pos		: クライアント番号
  ALL_CLIENTSが指定された時には全員に送る
  void	   *data	: 送るデータ
  int	   dataSize	: 送るデータのサイズ
  出力	: なし
 *****************************************************************/
void sendData(int pos, void *data, int dataSize) {
		assert(0 <= pos && pos < gClientNum || pos == ALL_CLIENTS); //送信先が正しいか確認
		assert(data != NULL); //データが存在するか確認
		assert(0 < dataSize); //データサイズの確認

		int	i;

		if (pos == ALL_CLIENTS) { //全員に送信する
				for (i = 0; i < gClientNum; i++) {
						write(gClients[i].fd,data,dataSize);
				}
		} else { //個人に送信する
				write(gClients[pos].fd,data,dataSize);
		}
}


void ending(void) {
		int i;

		printf("... Connection closed\n");
		for (i = 0; i < gClientNum; i++)
				close(gClients[i].fd); //クライアントを終了させる   
}  

/*****
  static
 *****/
static int multiAccept(int request_soc, int num) { //接続の確立
		/*
		   引数1:受付用ソケット
		   引数2:クライアント数
		 */
		int	i, j;
		int	fd;

		for (i=0; i < num; i++) {
				if ((fd = accept(request_soc,NULL,NULL)) == -1) { //接続
						fprintf(stderr,"Accept error\n");
						for (j = i - 1; j >= 0; j--)	close(gClients[j].fd);
						return -1;
				}
				enter(i, fd);
		}
		return fd;
}

static void enter(int pos, int fd) {
		/*
		   引数1:送信元
		   引数2:FD
		 */
		gClients[pos].fd = fd; //ファイルディスクリプタの格納
}

static void setMask(int maxfd) {
		int	i;

		gWidth = maxfd+1;

		FD_ZERO(&gMask);
		for (i = 0; i < gClientNum; i++)	FD_SET(gClients[i].fd, &gMask);
}

static void sendAllName() {
		int	i, j, tmp1, tmp2;

		tmp2 = htonl(gClientNum); //クライアント数のバイトオーダー
		for (i = 0; i < gClientNum; i++) {
				tmp1 = htonl(i);
				sendData(i, &tmp1, sizeof(int));
				sendData(i, &tmp2, sizeof(int)); //クライアント番号を振り分け
		}
}

static int recvData(int pos, void *data, int dataSize) {	// クライアントからデータを受け取る
		assert(0 <= pos && pos < gClientNum);
		assert(data != NULL);
		assert(0 < dataSize);

		return read(gClients[pos].fd, data, dataSize);
}
