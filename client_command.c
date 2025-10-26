/*****************************************************************
ファイル名	: client_command.c
機能		: クライアントのコマンド処理
*****************************************************************/

#include"common.h"
#include"client_func.h"
#include <arpa/inet.h> // htonl
#include <unistd.h> // read, write, close

static void SetIntData2DataBlock(void *data,int intData,int *dataSize);
static void SetCharData2DataBlock(void *data,char charData,int *dataSize);
// static void RecvCircleData(void); // 削除
// static void RecvRectangleData(void); // 削除
// static void RecvDiamondData(void); // 削除

/*****************************************************************
関数名	: ExecuteCommand
機能	: サーバーから送られてきたコマンドを元に，
		  引き数を受信し，実行する
引数	: char	command		: コマンド
出力	: プログラム終了コマンドがおくられてきた時には0を返す．
		  それ以外は1を返す
*****************************************************************/
int ExecuteCommand(char command)
{
    int	endFlag = 1;
#ifndef NDEBUG
    printf("#####\n");
    printf("ExecuteCommand()\n");
    printf("command = %c\n",command);
#endif
    switch(command){
		case END_COMMAND:
			endFlag = 0;
			break;
        /* 結果受信 */
	    case RESULT_WIN_COMMAND:
        case RESULT_LOSE_COMMAND:
        case RESULT_DRAW_COMMAND:
        case RESULT_WAIT_COMMAND: // 念のため残す
            DrawResult(command);
			break;
        
        /* (CIRCLE, RECT, DIAMOND は削除) */

        default:
            // 不明なコマンド
            printf("Unknown command received: %c\n", command);
            break;
    }
    return endFlag;
}

/* (SendRectangleCommand, SendCircleCommand は削除) */

/*****************************************************************
関数名	: SendJankenCommand
機能	: サーバーにじゃんけんの手を送信する
引数	: char handCommand (JANKEN_GOO_COMMAND など)
出力	: なし
*****************************************************************/
void SendJankenCommand(char handCommand)
{
    unsigned char	data[MAX_DATA];
    int			dataSize;

#ifndef NDEBUG
    printf("#####\n");
    printf("SendJankenCommand()\n");
    printf("Sending hand: %c\n", handCommand);
#endif
    dataSize = 0;
    /* コマンドのセット */
    SetCharData2DataBlock(data, handCommand, &dataSize);

    /* データの送信 */
    SendData(data,dataSize);
}


/*****************************************************************
関数名	: SendEndCommand
機能	: プログラムの終了を知らせるために，
		  サーバーにデータを送る
引数	: なし
出力	: なし
*****************************************************************/
void SendEndCommand(void)
{
    unsigned char	data[MAX_DATA];
    int			dataSize;

#ifndef NDEBUG
    printf("#####\n");
    printf("SendEndCommand()\n");
#endif
    dataSize = 0;
    /* コマンドのセット */
    SetCharData2DataBlock(data,END_COMMAND,&dataSize);

    /* データの送信 */
    SendData(data,dataSize);
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

    /* char 型のデータを送信用データの最後にコピーする */
    *(char *)(data + (*dataSize)) = charData;
    /* データサイズを増やす */
    (*dataSize) += sizeof(char);
}

/* (RecvCircleData, RecvRectangleData, RecvDiamondData は削除) */