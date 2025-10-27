/*****************************************************************
ファイル名	: client_command.c
機能		: クライアントのコマンド処理
*****************************************************************/

#include"common.h"
#include"client_func.h"
#include <arpa/inet.h> // htonl
#include <unistd.h> // read, write, close


static void SetCharData2DataBlock(void *data,char charData,int *dataSize);

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

        /* ★ 結果受信 (W, L, D は R に統合) */
	    case RESULT_DRAW_COMMAND: {
            int resultInt, opponentHandInt;
            char result, opponentHand;

            // サーバーから「結果(int)」と「相手の手(int)」を受信
            RecvIntData(&resultInt);
            RecvIntData(&opponentHandInt);
            
            result = (char)resultInt;
            opponentHand = (char)opponentHandInt;

#ifndef NDEBUG
            printf("Recv Result: %c, Opponent: %c\n", result, opponentHand);
#endif
            DrawResult(result, opponentHand); // client_win に描画を依頼
			break;
        }

        default:
            printf("Unknown command received: %c\n", command);
            break;
    }
    return endFlag;
}

/*****************************************************************
関数名	: SendJankenCommand
機能	: サーバーにじゃんけんの手を送信する
(中略)
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
    SetCharData2DataBlock(data, handCommand, &dataSize);
    SendData(data,dataSize);
}


/*****************************************************************
関数名	: SendEndCommand
機能	: プログラムの終了を知らせるために，
		  サーバーにデータを送る
(中略)
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
    SetCharData2DataBlock(data,END_COMMAND,&dataSize);
    SendData(data,dataSize);
}

/*****************************************************************
関数名	: SetCharData2DataBlock
機能	: char 型のデータを送信用データの最後にセットする
(中略)
*****************************************************************/
static void SetCharData2DataBlock(void *data,char charData,int *dataSize)
{
    assert(data!=NULL);
    assert(0<=(*dataSize));
    *(char *)(data + (*dataSize)) = charData;
    (*dataSize) += sizeof(char);
}