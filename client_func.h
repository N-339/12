/*****************************************************************
ファイル名	: client_func.h
機能		: クライアントの外部関数の定義
*****************************************************************/

#ifndef _CLIENT_FUNC_H_
#define _CLIENT_FUNC_H_

#include"common.h"

/* client_net.c */
extern int SetUpClient(char* hostName,int *clientID,int *num,char clientName[][MAX_NAME_SIZE]);
extern void CloseSoc(void);
extern int RecvIntData(int *intData);
extern void SendData(void *data,int dataSize);
extern int SendRecvManager(void);

/* client_win.c */
extern int InitWindows(int clientID,int num,char name[][MAX_NAME_SIZE]);
extern void DestroyWindow(void);
extern void WindowEvent(int num);
extern void DrawResult(char result); // 結果描画関数
// extern void DrawRectangle(int x,int y,int width,int height); // 削除
// extern void DrawCircle(int x,int y,int r); // 削除
// extern void DrawDiamond(int x,int y,int height); // 削除

/* client_command.c */
extern int ExecuteCommand(char command);
extern void SendJankenCommand(char handCommand); // じゃんけん送信
// extern void SendRectangleCommand(void); // 削除
// extern void SendCircleCommand(int pos); // 削除
extern void SendEndCommand(void);

#endif