/*****************************************************************
ファイル名	: server_command.c
機能		: サーバーのコマンド処理
*****************************************************************/

#include"server_common.h"
#include"server_func.h"
#include <arpa/inet.h> // htonl
#include <unistd.h> // read, write, close
#include <string.h> // memcpy を使うため追加

/* ★ SetIntData2DataBlock の宣言を追加 */
static void SetIntData2DataBlock(void *data,int intData,int *dataSize);
static void SetCharData2DataBlock(void *data,char charData,int *dataSize);

/* 2人のクライアントの手を保存する配列 (R, S, P を使用) */
static char gClientHands[MAX_CLIENTS] = {0, 0};

static void JudgeAndSendResult(void);
static const char* HandToText(char hand); // サーバー表示用ヘルパー

/*****************************************************************
関数名	: ExecuteCommand
機能	: クライアントから送られてきたコマンドを元に，
		  引き数を受信し，実行する
引数	: char	command		: コマンド
		  int	pos			: コマンドを送ったクライアント番号 (0 or 1)
出力	: プログラム終了コマンドが送られてきた時には0を返す．
		  それ以外は1を返す
*****************************************************************/
int ExecuteCommand(char command,int pos)
{
    unsigned char	data[MAX_DATA];
    int			dataSize;
    int			endFlag = 1;

    /* 引き数チェック */
    assert(0<=pos && pos<MAX_CLIENTS);

#ifndef NDEBUG
    printf("#####\n");
    printf("ExecuteCommand()\n");
    printf("Client %d sent command %c\n", pos, command);
#endif
    switch(command){
	    case END_COMMAND:
			dataSize = 0;
			/* コマンドのセット */
			SetCharData2DataBlock(data,command,&dataSize);

			/* 全ユーザーに送る */
			SendData(ALL_CLIENTS,data,dataSize);

			endFlag = 0;
			break;
        
        /* じゃんけんの手を受信 (R, S, P) */
	    case JANKEN_ROCK_COMMAND:
        case JANKEN_SCISSORS_COMMAND:
        case JANKEN_PAPER_COMMAND:
            if (gClientHands[pos] != 0) {
#ifndef NDEBUG
                printf("Client %d already sent hand. Ignoring.\n", pos);
#endif
                break;
            }
            gClientHands[pos] = command;

            if (gClientHands[0] != 0 && gClientHands[1] != 0) {
                JudgeAndSendResult();
            } else {
#ifndef NDEBUG
                printf("Waiting for other client's hand.\n");
#endif
            }
			break;
        
	    default:
			/* 未知のコマンドが送られてきた */
			fprintf(stderr,"Client %d sent unknown command: 0x%02x\n", pos, command);
    }
    return endFlag;
}


/*****
static
*****/

/*****************************************************************
関数名	: HandToText
機能	: 手の文字 (R,S,P) を文字列 ("Rock" 等) に変換する
引数	: char hand
出力	: const char*
*****************************************************************/
static const char* HandToText(char hand)
{
    switch(hand) {
        case JANKEN_ROCK_COMMAND:    return "Rock";
        case JANKEN_SCISSORS_COMMAND:  return "Scissors";
        case JANKEN_PAPER_COMMAND:    return "Paper";
        default:                    return "Unknown";
    }
}

/*****************************************************************
関数名	: JudgeAndSendResult
機能	: 2人の手を判定し，結果を両クライアントに送信する
引数	: なし
出力	: なし
*****************************************************************/
static void JudgeAndSendResult(void)
{
    char hand0 = gClientHands[0];
    char hand1 = gClientHands[1];
    char result0, result1;
    const char* resultStr;

#ifndef NDEBUG
    printf("Judging: Client 0 [%c] vs Client 1 [%c]\n", hand0, hand1);
#endif

    /* 判定ロジック (R, S, P を使用) */
    if (hand0 == hand1) {
        result0 = RESULT_DRAW_COMMAND;
        result1 = RESULT_DRAW_COMMAND;
        resultStr = "DRAW";
    } else if ( (hand0 == JANKEN_ROCK_COMMAND && hand1 == JANKEN_SCISSORS_COMMAND) ||
                (hand0 == JANKEN_SCISSORS_COMMAND && hand1 == JANKEN_PAPER_COMMAND) ||
                (hand0 == JANKEN_PAPER_COMMAND && hand1 == JANKEN_ROCK_COMMAND) ) {
        result0 = RESULT_WIN_COMMAND;
        result1 = RESULT_LOSE_COMMAND;
        resultStr = "Client 0 WIN";
    } else {
        result0 = RESULT_LOSE_COMMAND;
        result1 = RESULT_WIN_COMMAND;
        resultStr = "Client 1 WIN";
    }

    /* サーバー側に結果を表示 */
    printf("Server Result: C0(%s) vs C1(%s) -> %s\n", 
           HandToText(hand0), HandToText(hand1), resultStr);

    /* ★ 修正: クライアントに 'A' コマンド +「結果(int)」+「相手の手(int)」を送信 */

    // --- Client 0 に結果送信 (結果 + 相手(1)の手) ---
    unsigned char data0[MAX_DATA];
    int dataSize0 = 0;
    SetCharData2DataBlock(data0, RESULT_DATA_COMMAND, &dataSize0); // 'A' コマンド
    SetIntData2DataBlock(data0, (int)result0, &dataSize0);         // W, L, or D
    SetIntData2DataBlock(data0, (int)hand1, &dataSize0);           // 相手の手 (R, S, P)
    SendData(0, data0, dataSize0); 

    // --- Client 1 に結果送信 (結果 + 相手(0)の手) ---
    unsigned char data1[MAX_DATA];
    int dataSize1 = 0;
    SetCharData2DataBlock(data1, RESULT_DATA_COMMAND, &dataSize1); // 'A' コマンド
    SetIntData2DataBlock(data1, (int)result1, &dataSize1);         // W, L, or D
    SetIntData2DataBlock(data1, (int)hand0, &dataSize1);           // 相手の手 (R, S, P)
    SendData(1, data1, dataSize1); 

    gClientHands[0] = 0;
    gClientHands[1] = 0;
}

/* ★ SetIntData2DataBlock の定義を追加 */
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
    assert(data!=NULL);
    assert(0<=(*dataSize));
    tmp = htonl(intData);
    memcpy(data + (*dataSize),&tmp,sizeof(int));
    (*dataSize) += sizeof(int);
}

/*****************************************************************
関数名	: SetCharData2DataBlock
機能	: char 型のデータを送信用データの最後にセットする
引数	: void		*data		: 送信用データ
		  char		charData	: セットするデータ
		  int		*dataSize	: 送信用データの現在のサイズ
出力	: なし
*****************************************************************/
static void SetCharData2DataBlock(void *data,char charData,int *dataSize)
{
    assert(data!=NULL);
    assert(0<=(*dataSize));
    *(char *)(data + (*dataSize)) = charData;
    (*dataSize) += sizeof(char);
}