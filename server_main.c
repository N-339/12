/*****************************************************************
ファイル名	: server_main.c
機能		: サーバーのメインルーチン
*****************************************************************/

// #include<SDL2/SDL.h> // SDLタイマーを使わないので不要
#include"server_common.h"
#include"server_func.h" // Ending, SendRecvManager, SetUpServer

// static Uint32 SignalHandler(Uint32 interval, void *param); // 削除

int main(int argc,char *argv[])
{
	int	num;
	int	endFlag = 1;

	/* 引き数チェック (クライアント数を2に固定) */
	if(argc != 1){ // 引数不要
		fprintf(stderr,"Usage: %s (No arguments required. Clients = 2)\n", argv[0]);
		// exit(-1); // 警告のみで続行してもよい
	}
    
    num = MAX_CLIENTS; // common.h で 2 に定義
	
	/* (SDLの初期化は削除) */

	/* クライアントとの接続 */
	if(SetUpServer(num) == -1){
		fprintf(stderr,"Cannot setup server\n");
		exit(-1);
	}
	
	/* (割り込み処理のセットは削除) */
	
    printf("Janken server started. Waiting for %d clients...\n", num);

	/* メインイベントループ */
	while(endFlag){
		endFlag = SendRecvManager();
	};

	/* 終了処理 */
	Ending();

	return 0;
}

/* (SignalHandler は削除) */