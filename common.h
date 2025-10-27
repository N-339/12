/*****************************************************************
ファイル名	: common.h
機能		: サーバーとクライアントで使用する定数の宣言を行う
*****************************************************************/

#ifndef _COMMON_H_
#define _COMMON_H_

#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include<sys/types.h>
#include<assert.h>
#include<math.h>
// #include<unistd.h> // client_net.c, server_net.c で read/write/close のために必要
// #include<netinet/in.h> // client_net.c, server_net.c で必要
// #include<arpa/inet.h> // client_command.c, server_command.c で htonl/ntohl のために必要

#define PORT			(u_short)8888	/* ポート番号 */

#define MAX_CLIENTS		2				/* クライアント数の最大値 (2に固定) */
#define MAX_NAME_SIZE	10 				/* ユーザー名の最大値*/

#define MAX_DATA		200				/* 送受信するデータの最大値 */

/* コマンド定義 */
#define END_COMMAND		'E'		  		/* プログラム終了コマンド */

/* じゃんけんの手コマンド (クライアント -> サーバー) */
#define JANKEN_ROCK_COMMAND	'R'			/* グー */
#define JANKEN_SCISSORS_COMMAND	'S'			/* チョキ */
#define JANKEN_PAPER_COMMAND	'P'			/* パー */

#define RESULT_DATA_COMMAND     'A'         /* 結果データ */

/* じゃんけんの結果コマンド (サーバー -> クライアント) */
#define RESULT_WIN_COMMAND	'W'			/* 勝ち */
#define RESULT_LOSE_COMMAND	'L'			/* 負け */
#define RESULT_DRAW_COMMAND	'D'			/* あいこ */

#endif