/*****************************************************************
ファイル名	: client_command.c
機能		: クライアントのコマンド処理
*****************************************************************/

#include"common.h"
#include"client_func.h"
// #include <arpa/inet.h> // SetIntData2DataBlockを使わないので不要
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

        /* ★ 修正: 'D' ではなく 'A' (RESULT_DATA_COMMAND) を受信する */
	    case RESULT_DATA_COMMAND: { 
            int resultInt, opponentHandInt;
            char result, opponentHand;

            // サーバーから「結果(int)」と「相手の手(int)」を追加で受信
            RecvIntData(&resultInt);
            RecvIntData(&opponentHandInt);
            
            result = (char)resultInt;       // W, L, D のいずれか
            opponentHand = (char)opponentHandInt; // R, S, P のいずれか

#ifndef NDEBUG
            printf("Recv Result: %c, Opponent: %c\n", result, opponentHand);
#endif
            DrawResult(result, opponentHand); // client_win に描画を依頼
			break;
        }

        default:
            /* これで 'R', 'S', 'W', 'L', 'D' などは 'Unknown' として扱われる (デバッグに役立つ) */
            printf("Unknown command received: %c (0x%x)\n", command, command);
            break;
    }
    return endFlag;
}

/*****************************************************************
関数名	: SendJankenCommand
機能	: サーバーにじゃんけんの手を送信する
引数	: char handCommand (R, S, P)
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
    SetCharData2DataBlock(data, handCommand, &dataSize);
    SendData(data,dataSize);
}


/*****************************************************************
関数名	: SendEndCommand
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
(中略)
*****************************************************************/
static void SetCharData2DataBlock(void *data,char charData,int *dataSize)
{
    assert(data!=NULL);
    assert(0<=(*dataSize));
    *(char *)(data + (*dataSize)) = charData;
    (*dataSize) += sizeof(char);
}