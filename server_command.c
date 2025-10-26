/*****************************************************************
ファイル名	: server_command.c
機能		: サーバーのコマンド処理
*****************************************************************/

#include"server_common.h"
#include"server_func.h"
#include <arpa/inet.h> // htonl
#include <unistd.h> // read, write, close

static void SetIntData2DataBlock(void *data,int intData,int *dataSize);
static void SetCharData2DataBlock(void *data,char charData,int *dataSize);
// static int GetRandomInt(int n); // 乱数を使わないので削除

/* 2人のクライアントの手を保存する配列 */
/* 0 = 未受信, G = グー, C = チョキ, P = パー */
static char gClientHands[MAX_CLIENTS] = {0, 0};

static void JudgeAndSendResult(void);

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
        
        /* じゃんけんの手を受信 */
	    case JANKEN_GOO_COMMAND:
        case JANKEN_CHOKI_COMMAND:
        case JANKEN_PAR_COMMAND:
            // 既に手を受信済みの場合は無視 (クライアント側で制御してるはずだが念のため)
            if (gClientHands[pos] != 0) {
#ifndef NDEBUG
                printf("Client %d already sent hand. Ignoring.\n", pos);
#endif
                break;
            }

            // 手を保存
            gClientHands[pos] = command;

            // 2人分の手が揃ったかチェック
            if (gClientHands[0] != 0 && gClientHands[1] != 0) {
                // 揃ったら判定して結果送信
                JudgeAndSendResult();
            } else {
                // 揃ってない場合 (片方だけが送信した)
                // (クライアント側で「相手待ち」を表示しているのでサーバーからは送らない)
#ifndef NDEBUG
                printf("Waiting for other client's hand.\n");
#endif
            }
			break;
        
        /* (CIRCLE, RECT は削除) */

	    default:
			/* 未知のコマンドが送られてきた */
			fprintf(stderr,"Client %d sent unknown command: 0x%02x\n", pos, command);
    }
    return endFlag;
}

/* (SendDiamondCommand は削除) */


/*****
static
*****/

/*****************************************************************
関数名	: JudgeAndSendResult
機能	: 2人の手を判定し，結果を両クライアントに送信する
引数	: なし (static変数 gClientHands を参照)
出力	: なし
*****************************************************************/
static void JudgeAndSendResult(void)
{
    char hand0 = gClientHands[0];
    char hand1 = gClientHands[1];
    char result0, result1; // Client 0 と 1 の結果

#ifndef NDEBUG
    printf("Judging: Client 0 [%c] vs Client 1 [%c]\n", hand0, hand1);
#endif

    if (hand0 == hand1) {
        // あいこ
        result0 = RESULT_DRAW_COMMAND;
        result1 = RESULT_DRAW_COMMAND;
    } else if ( (hand0 == JANKEN_GOO_COMMAND && hand1 == JANKEN_CHOKI_COMMAND) ||
                (hand0 == JANKEN_CHOKI_COMMAND && hand1 == JANKEN_PAR_COMMAND) ||
                (hand0 == JANKEN_PAR_COMMAND && hand1 == JANKEN_GOO_COMMAND) ) {
        // Client 0 の勝ち
        result0 = RESULT_WIN_COMMAND;
        result1 = RESULT_LOSE_COMMAND;
    } else {
        // Client 1 の勝ち (上記以外の組み合わせ)
        result0 = RESULT_LOSE_COMMAND;
        result1 = RESULT_WIN_COMMAND;
    }

    // --- Client 0 に結果送信 ---
    unsigned char data0[MAX_DATA];
    int dataSize0 = 0;
    SetCharData2DataBlock(data0, result0, &dataSize0);
    SendData(0, data0, dataSize0); // Client 0 へ送信

    // --- Client 1 に結果送信 ---
    unsigned char data1[MAX_DATA];
    int dataSize1 = 0;
    SetCharData2DataBlock(data1, result1, &dataSize1);
    SendData(1, data1, dataSize1); // Client 1 へ送信

#ifndef NDEBUG
    printf("Result sent: Client 0 [%c], Client 1 [%c]\n", result0, result1);
#endif

    // 次の勝負のために手をリセット
    gClientHands[0] = 0;
    gClientHands[1] = 0;
}


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

/* (GetRandomInt は削除) */