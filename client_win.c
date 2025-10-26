/*****************************************************************
ファイル名	: client_win.c
機能		: クライアントのユーザーインターフェース処理
*****************************************************************/

#include<SDL2/SDL.h>
#include<SDL2/SDL_image.h>
#include<SDL2/SDL2_gfxPrimitives.h>
#include"common.h"
#include"client_func.h"

static SDL_Window *gMainWindow;
static SDL_Renderer *gMainRenderer;
static SDL_Rect gButtonRect[4]; // グー、チョキ、パー、END の4つ

// サーバーからの結果待ちかどうか (0: 待ってない, 1: 待ってる)
static int gWaitingForResult = 0;

static int CheckButtonNO(int x,int y);
static void DrawResultText(const char* text);

/*****************************************************************
関数名	: InitWindows
機能	: メインウインドウの表示，設定を行う
引数	: int	clientID		: クライアント番号
		  int	num				: 全クライアント数 (常に2のはず)
出力	: 正常に設定できたとき0，失敗したとき-1
*****************************************************************/
int InitWindows(int clientID,int num,char name[][MAX_NAME_SIZE])
{
	int i;
	SDL_Texture *texture;
	SDL_Surface *image;
	SDL_Rect src_rect;
	// グー、チョキ、パー、END
	char buttonFiles[4][10]={"1.jpg","2.jpg","3.jpg","END.jpg"}; 
	char *s,title[10];

    /* SDLの初期化 */
	if(SDL_Init(SDL_INIT_VIDEO) < 0) {
		printf("failed to initialize SDL.\n");
		return -1;
	}
	
	/* メインのウインドウを作成する (サイズを調整 340x150) */
	if((gMainWindow = SDL_CreateWindow("Janken Game", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, 340, 150, 0)) == NULL) {
		printf("failed to initialize videomode.\n");
		return -1;
	}

	gMainRenderer = SDL_CreateRenderer(gMainWindow, -1, SDL_RENDERER_SOFTWARE);

	/* ウインドウのタイトルをセット (クライアントID) */
	sprintf(title,"Client %d",clientID);
	SDL_SetWindowTitle(gMainWindow, title);
	
	/* 背景を白にする */
	SDL_SetRenderDrawColor(gMainRenderer, 255, 255, 255, 255);
  	SDL_RenderClear(gMainRenderer);

	/* ボタンの作成 (4つ) */
	for(i=0;i<4;i++){
		gButtonRect[i].x = 20 + 80*i; // 80間隔
		gButtonRect[i].y = 10;
		gButtonRect[i].w = 70;
		gButtonRect[i].h = 50; // 少し高さを確保
      
		s = buttonFiles[i];

		image = IMG_Load(s);
        if (image == NULL) {
            printf("failed to load image: %s (SDL_image Error: %s)\n", s, IMG_GetError());
            // 1.jpg, 2.jpg, 3.jpg, END.jpg が必要
            SDL_DestroyRenderer(gMainRenderer);
            SDL_DestroyWindow(gMainWindow);
            SDL_Quit();
            return -1;
        }
		texture = SDL_CreateTextureFromSurface(gMainRenderer, image);
		src_rect = (SDL_Rect){0, 0, image->w, image->h};
        // ボタンの矩形に合わせて描画
		SDL_RenderCopy(gMainRenderer, texture, &src_rect, (&gButtonRect[i]));
		SDL_FreeSurface(image);
        SDL_DestroyTexture(texture);
	}

    /* 結果表示領域の初期化 */
    DrawResultText("Welcome! Choose your hand.");

	SDL_RenderPresent(gMainRenderer);
	
    gWaitingForResult = 0; // 初期状態は待っていない
	return 0;
}

/*****************************************************************
関数名	: DestroyWindow
機能	: SDLを終了する
引数	: なし
出力	: なし
*****************************************************************/
void DestroyWindow(void)
{
    SDL_DestroyRenderer(gMainRenderer);
    SDL_DestroyWindow(gMainWindow);
	SDL_Quit();
}

/*****************************************************************
関数名	: WindowEvent
機能	: メインウインドウに対するイベント処理を行う
引数	: int		num		: 全クライアント数 (未使用だがIF維持)
出力	: なし
*****************************************************************/
void WindowEvent(int num)
{
	SDL_Event event;
	SDL_MouseButtonEvent *mouse;
	int buttonNO;

	if(SDL_PollEvent(&event)){

		switch(event.type){
			case SDL_QUIT:
                // ウィンドウ閉じたらEND送信
				SendEndCommand();
				break;
			case SDL_MOUSEBUTTONUP:
				mouse = (SDL_MouseButtonEvent*)&event;
				if(mouse->button == SDL_BUTTON_LEFT){
					
                    // サーバーからの結果待ち中はボタン操作を無視
                    if (gWaitingForResult) {
#ifndef NDEBUG
                        printf("Waiting for result. Button press ignored.\n");
#endif
                        break;
                    }

					buttonNO = CheckButtonNO(mouse->x,mouse->y);
#ifndef NDEBUG
					printf("#####\n");
					printf("WindowEvent()\n");
					printf("Button %d is pressed\n",buttonNO);
#endif
					switch(buttonNO) {
                        case 0: // グー
                            SendJankenCommand(JANKEN_GOO_COMMAND);
                            gWaitingForResult = 1; // 結果待ち状態へ
                            DrawResultText("You: GOO. Waiting for opponent...");
                            break;
                        case 1: // チョキ
                            SendJankenCommand(JANKEN_CHOKI_COMMAND);
                            gWaitingForResult = 1; // 結果待ち状態へ
                            DrawResultText("You: CHOKI. Waiting for opponent...");
                            break;
                        case 2: // パー
                            SendJankenCommand(JANKEN_PAR_COMMAND);
                            gWaitingForResult = 1; // 結果待ち状態へ
                            DrawResultText("You: PAR. Waiting for opponent...");
                            break;
                        case 3: // END
                            SendEndCommand();
                            // END送信時はgWaitingForResultを操作しない (そのまま終了するため)
                            break;
                    }
				}
				break;
		}
	}
}

/*****************************************************************
関数名	: DrawResult
機能	: メインウインドウにじゃんけんの結果を表示する
引数	: char	result		: 結果コマンド (W, L, D)
出力	: なし
*****************************************************************/
void DrawResult(char result)
{
    gWaitingForResult = 0; // 結果が来たので待ち状態解除

    switch(result) {
        case RESULT_WIN_COMMAND:
            DrawResultText("You WIN! Choose next hand.");
            break;
        case RESULT_LOSE_COMMAND:
            DrawResultText("You LOSE... Choose next hand.");
            break;
        case RESULT_DRAW_COMMAND:
            DrawResultText("DRAW. Choose next hand.");
            break;
        case RESULT_WAIT_COMMAND:
            // サーバー側ロジック変更により、これは使われないはず
            // (クライアント側で「相手待ち」を表示しているため)
             DrawResultText("Waiting for opponent...");
             gWaitingForResult = 1; // 相手待ちならまだ待つ
            break;
    }
}


/* (DrawRectangle, DrawCircle, DrawDiamond は削除) */


/*****
static
*****/

/*****************************************************************
関数名	: DrawResultText
機能	: ウィンドウ下部に結果文字列を描画する
引数	: const char* text : 表示する文字列
出力	: なし
*****************************************************************/
static void DrawResultText(const char* text)
{
    // 下部の領域を白でクリア
    SDL_Rect clearRect = {0, 80, 340, 70}; // Y=80から下
	SDL_SetRenderDrawColor(gMainRenderer, 255, 255, 255, 255);
  	SDL_RenderFillRect(gMainRenderer, &clearRect);

    // テキストを描画 (黒色: 0x000000ff)
    // 座標は (X=10, Y=100)
    stringColor(gMainRenderer, 10, 100, text, 0x000000ff);

    SDL_RenderPresent(gMainRenderer);
}


/*****************************************************************
関数名	: CheckButtonNO
機能	: クリックされたボタンの番号を返す
引数	: int	   x		: マウスの押された x 座標
		  int	   y		: マウスの押された y 座標
出力	: 押されたボタンの番号(0-3)を返す
		  ボタンが押されていない時は-1を返す
*****************************************************************/
static int CheckButtonNO(int x,int y)
{
	int i;
    int num = 4; // ボタンは4つ

 	for(i=0;i<num;i++){
		if(gButtonRect[i].x < x &&
			gButtonRect[i].y < y &&
      		gButtonRect[i].x + gButtonRect[i].w > x &&
			gButtonRect[i].y + gButtonRect[i].h > y){
			return i;
		}
	}
 	return -1;
}