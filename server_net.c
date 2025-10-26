/*****************************************************************
ファイル名	: server_net.c
機能		: サーバーのネットワーク処理
*****************************************************************/

#include"server_common.h"
#include"server_func.h"
#include<sys/socket.h>
#include<netinet/in.h>
#include<netdb.h>
#include <unistd.h> // read, write, close
#include <arpa/inet.h> // ntohl, htonl

/* クライアントを表す構造体 */
typedef struct{
	int		fd;
	char	name[MAX_NAME_SIZE];
}CLIENT;

static CLIENT	gClients[MAX_CLIENTS];	/* クライアント */
static int	gClientNum;					/* クライアント数 */

static fd_set	gMask;					/* select()用のマスク */
static int	gWidth;						/* gMask中のチェックすべきビット数 */

static int MultiAccept(int request_soc,int num);
static void Enter(int pos, int fd);
static void SetMask(int maxfd);
static void SendAllName(void);
static int RecvData(int pos,void *data,int dataSize);

/*****************************************************************
関数名	: SetUpServer
機能	: クライアントとのコネクションを設立し，
		  ユーザーの名前の送受信を行う
引数	: int		num		  : クライアント数
出力	: コネクションに失敗した時-1,成功した時0
*****************************************************************/
int SetUpServer(int num)
{
    struct sockaddr_in	server;
    int			request_soc;
    int                 maxfd;
    int			val = 1;
 
    /* 引き数チェック */
    assert(0<num && num<=MAX_CLIENTS);

    gClientNum = num;
    bzero((char*)&server,sizeof(server));
    server.sin_family = AF_INET;
    server.sin_port = htons(PORT);
    server.sin_addr.s_addr = htonl(INADDR_ANY);

    /* ソケットを作成する */
    if((request_soc = socket(AF_INET,SOCK_STREAM,0)) < 0){
		fprintf(stderr,"Socket allocation failed\n");
		return -1;
    }
    setsockopt(request_soc,SOL_SOCKET,SO_REUSEADDR,&val,sizeof(val));

    /* ソケットに名前をつける */
    if(bind(request_soc,(struct sockaddr*)&server,sizeof(server))==-1){
		fprintf(stderr,"Cannot bind\n");
		close(request_soc);
		return -1;
    }
    fprintf(stderr,"Successfully bind!\n");
    
    /* クライアントからの接続要求を待つ */
    if(listen(request_soc, gClientNum) == -1){
		fprintf(stderr,"Cannot listen\n");
		close(request_soc);
		return -1;
    }
    fprintf(stderr,"Listen OK\n");

    /* クライアントと接続する */
    maxfd = MultiAccept(request_soc, gClientNum);
    close(request_soc);
    if(maxfd == -1)return -1;

    /* 全クライアントの全ユーザー名を送る */
    SendAllName();

    /* select()のためのマスク値を設定する */
    SetMask(maxfd);

    return 0;
}

/*****************************************************************
関数名	: SendRecvManager
機能	: サーバーから送られてきたデータを処理する
引数	: なし
出力	: プログラム終了コマンドが送られてきた時0を返す．
		  それ以外は1を返す
*****************************************************************/
int SendRecvManager(void)
{
    char	command;
    fd_set	readOK;
    int		i;
    int		endFlag = 1;

    readOK = gMask;
    /* クライアントからデータが届いているか調べる */
    if(select(gWidth,&readOK,NULL,NULL,NULL) < 0){
        /* エラーが起こった */
         perror("select"); // エラー内容表示
        return endFlag;
    }

    for(i=0;i<gClientNum;i++){
		if(FD_ISSET(gClients[i].fd,&readOK)){
	    	/* クライアントからデータが届いていた */
	    	/* コマンドを読み込む */
			int n = RecvData(i,&command,sizeof(char));
            if (n <= 0) {
                // クライアントが切断した
                fprintf(stderr, "Client %d disconnected.\n", i);
                command = END_COMMAND; // 終了コマンドとして扱う
            }
	    	/* コマンドに対する処理を行う */
	    	endFlag = ExecuteCommand(command,i);
	    	if(endFlag == 0)break;
		}
    }
    return endFlag;
}

/*****************************************************************
関数名	: RecvIntData
機能	: クライアントからint型のデータを受け取る
引数	: int		pos	        : クライアント番号
		  int		*intData	: 受信したデータ
出力	: 受け取ったバイト数 (エラー時は 0 or -1)
*****************************************************************/
int RecvIntData(int pos,int *intData)
{
    int n,tmp;
    
    /* 引き数チェック */
    assert(0<=pos && pos<gClientNum);
    assert(intData!=NULL);

    n = RecvData(pos,&tmp,sizeof(int));
    if (n == sizeof(int)) { // 正常にintサイズ受信できた時のみ変換
        (*intData) = ntohl(tmp);
    }
    
    return n;
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
void SendData(int pos,void *data,int dataSize)
{
    int	i;
   
    /* 引き数チェック */
    assert((0<=pos && pos<gClientNum) || pos==ALL_CLIENTS);
    assert(data!=NULL);
    assert(0<dataSize);

    if(pos == ALL_CLIENTS){
    	/* 全クライアントにデータを送る */
		for(i=0;i<gClientNum;i++){
			write(gClients[i].fd,data,dataSize);
		}
    }
    else{
		write(gClients[pos].fd,data,dataSize);
    }
}

/*****************************************************************
関数名	: Ending
機能	: 全クライアントとのコネクションを切断する
引数	: なし
出力	: なし
*****************************************************************/
void Ending(void)
{
    int	i;

    printf("... Connection closed\n");
    for(i=0;i<gClientNum;i++)close(gClients[i].fd);
}

/*****
static
*****/
/*****************************************************************
関数名	: MultiAccept
機能	: 接続要求のあったクライアントとのコネクションを設立する
引数	: int		request_soc	: ソケット
		  int		num     	: クライアント数
出力	: ソケットディスクリプタ (最後に追加されたFD)
*****************************************************************/
static int MultiAccept(int request_soc,int num)
{
    int	i,j;
    int	fd;
    
    for(i=0;i<num;i++){
        printf("Waiting for client %d...\n", i); // 待ち受け中表示
		if((fd = accept(request_soc,NULL,NULL)) == -1){
			fprintf(stderr,"Accept error\n");
			for(j=i-1;j>=0;j--)close(gClients[j].fd);
			return -1;
		}
        printf("Client %d accepted (fd=%d).\n", i, fd);
		Enter(i,fd);
    }
    return fd; // 最後にacceptしたfdを返す
}

/*****************************************************************
関数名	: Enter
機能	: クライアントのユーザー名を受信する
引数	: int		pos		: クライアント番号
		  int		fd		: ソケットディスクリプタ
出力	: なし
*****************************************************************/
static void Enter(int pos, int fd)
{
	/* クライアントのユーザー名を受信する */
	read(fd,gClients[pos].name,MAX_NAME_SIZE);
#ifndef NDEBUG
	printf("%s is accepted (pos=%d)\n",gClients[pos].name, pos);
#endif
	gClients[pos].fd = fd;
}

/*****************************************************************
関数名	: SetMask
機能	: int		maxfd	: ソケットディスクリプタの最大値
引数	: なし
出力	: なし
*****************************************************************/
static void SetMask(int maxfd)
{
    int	i;

    // maxfd ではなく、FD の最大値を再計算する
    // (MultiAccept は最後に接続したFDしか返さないため)
    gWidth = 0; 
    FD_ZERO(&gMask);    
    for(i=0;i<gClientNum;i++){
        FD_SET(gClients[i].fd,&gMask);
        if (gClients[i].fd > gWidth) {
            gWidth = gClients[i].fd;
        }
    }
    gWidth++; // select の width は (最大FD + 1)
}

/*****************************************************************
関数名	: SendAllName
機能	: 全クライアントに全ユーザー名を送る
引数	: なし
出力	: なし
*****************************************************************/
static void SendAllName(void)
{
  int	i,j,tmp1,tmp2;

    tmp2 = htonl(gClientNum);
    for(i=0;i<gClientNum;i++){
		tmp1 = htonl(i); // クライアントID
		SendData(i,&tmp1,sizeof(int));
		SendData(i,&tmp2,sizeof(int));
		for(j=0;j<gClientNum;j++){
			SendData(i,gClients[j].name,MAX_NAME_SIZE);
		}
	}
}

/*****************************************************************
関数名	: RecvData
機能	: クライアントからデータを受け取る
引数	: int		pos	        : クライアント番号
		  void		*data		: 受信したデータ
		  int		dataSize	: 受信するデータのサイズ
出力	: 受け取ったバイト数 (切断・エラー時は 0 or -1)
*****************************************************************/
static int RecvData(int pos,void *data,int dataSize)
{
    int n;
    
    /* 引き数チェック */
    assert(0<=pos && pos<gClientNum);
    assert(data!=NULL);
    assert(0<dataSize);

    n = read(gClients[pos].fd,data,dataSize);
    
    return n;
}