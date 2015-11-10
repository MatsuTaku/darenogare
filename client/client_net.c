#include "../common.h"
#include "client_common.h"
#include "client_func.h"
#include <sys/socket.h>
#include <netdb.h>

#define	BUF_SIZE	100

static int	gSocket;	
static fd_set	gMask;	
static int	gWidth;		

static void getAllName(int *clientID,int *num);
static void setMask(void);

int setUpClient(char *hostName,int *clientID,int *num)
{
    struct hostent	*servHost;
    struct sockaddr_in	server;
    int			len;
    char		str[BUF_SIZE];

    if((servHost = gethostbyname(hostName))==NULL){
		fprintf(stderr,"Unknown host\n");
		return -1;
    }

    bzero((char*)&server,sizeof(server));
    server.sin_family = AF_INET;
    server.sin_port = htons(PORT);
    bcopy(servHost->h_addr,(char*)&server.sin_addr,servHost->h_length);

    if((gSocket = socket(AF_INET,SOCK_STREAM,0)) < 0){
		fprintf(stderr,"socket allocation failed\n");
		return -1;
    }

    if(connect(gSocket,(struct sockaddr*)&server,sizeof(server)) == -1){
		fprintf(stderr,"cannot connect\n");
		close(gSocket);
		return -1;
    }
    fprintf(stderr,"connected\n");

    do {
		printf("Enter Your Name\n");
		fgets(str,BUF_SIZE,stdin);
		len = strlen(str)-1;
		str[len]='\0';
    } while(len>MAX_NAME_SIZE-1 || len==0);
    sendData(str, MAX_NAME_SIZE);

    printf("Please Wait\n");

    getAllName(clientID,num);

    setMask();
    
    return 0;
}

int sendRecvManager(void)
{
    fd_set	readOK;
	int command;
    int		i;
    int		endFlag = 1;
    struct timeval	timeout;

    timeout.tv_sec = 0;
    timeout.tv_usec = 20;

    readOK = gMask;
    select(gWidth,&readOK,NULL,NULL,&timeout);
    if(FD_ISSET(gSocket,&readOK)){
		recvData(&command,sizeof(int));
		endFlag = executeCommand(command);
    }
    return endFlag;
}

int recvIntData(int *intData)
{
    int n,tmp;
    
    assert(intData!=NULL);

    n = recvData(&tmp,sizeof(int));
    (*intData) = ntohl(tmp);
    
    return n;
}

void sendData(void *data, int dataSize)
{
    assert(data != NULL);
    assert(0 < dataSize);

    write(gSocket,data,dataSize);
}

void closeSoc(void) //ソケットを閉じる
{
    printf("...Connection closed\n");
    close(gSocket);
}

/*****
static
*****/
static void getAllName(int *clientID,int *num)
{
    int	i;

    recvIntData(clientID);
    recvIntData(num);
}

static void setMask(void)
{
    int	i;

    FD_ZERO(&gMask);
    FD_SET(gSocket,&gMask);

    gWidth = gSocket+1;
}

int recvData(void *data,int dataSize)
{
    assert(data != NULL);
    assert(0 < dataSize);

    return read(gSocket,data,dataSize);
}
