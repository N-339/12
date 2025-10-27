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

static int gWaitingForResult = 0; // サーバーからの結果待ち

static char gMyHand = 0;
static char gOpponentHand = 0;

/* じゃんけん画像表示用のテクスチャと矩形 */
static SDL_Texture *gMyHandTexture = NULL;
static SDL_Texture *gOpponentHandTexture = NULL;
static SDL_Rect gMyHandImageRect;
static SDL_Rect gOpponentHandImageRect;


static int CheckButtonNO(int x,int y);
static void DrawGameStatus(const char* resultMsg, char myHandChar, char oppHandChar);
static SDL_Texture* LoadHandTexture(char handChar);
static void ClearResultImages(void);


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
    /*画像ファイル名を変更 */
	char buttonFiles[4][10]={"R.png","S.png","P.png","END.png"}; 
	char *s,title[10];

    /* SDLの初期化 */
	if(SDL_Init(SDL_INIT_VIDEO) < 0) {
		printf("failed to initialize SDL.\n");
		return -1;
	}
	
	/* メインのウインドウを作成する (サイズを調整 3倍) */
	if((gMainWindow = SDL_CreateWindow("Janken Game", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, 1020, 450, 0)) == NULL) {
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
		gButtonRect[i].x = 60 + 240*i; 
		gButtonRect[i].y = 30;         
		gButtonRect[i].w = 210;        
		gButtonRect[i].h = 150;        
      
		s = buttonFiles[i];

		image = IMG_Load(s);
        if (image == NULL) {
            printf("failed to load image: %s (SDL_image Error: %s)\n", s, IMG_GetError());
            SDL_DestroyRenderer(gMainRenderer);
            SDL_DestroyWindow(gMainWindow);
            SDL_Quit();
            return -1;
        }
		texture = SDL_CreateTextureFromSurface(gMainRenderer, image);
		src_rect = (SDL_Rect){0, 0, image->w, image->h};
		SDL_RenderCopy(gMainRenderer, texture, &src_rect, (&gButtonRect[i]));
		SDL_FreeSurface(image);
        SDL_DestroyTexture(texture);
	}

    /* じゃけん画像表示領域の矩形を初期化 (ボタンの下あたり) */
    gMyHandImageRect = (SDL_Rect){ 600, 250, 150, 150 }; // 右下、左寄り
    gOpponentHandImageRect = (SDL_Rect){ 800, 250, 150, 150 }; // 右下、右寄り

    DrawGameStatus("Welcome! Choose your hand.", 0, 0); // 自分の手と相手の手は最初は表示しない

	SDL_RenderPresent(gMainRenderer);
	
    gWaitingForResult = 0; 
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
    /* テクスチャの解放 */
    if (gMyHandTexture) SDL_DestroyTexture(gMyHandTexture);
    if (gOpponentHandTexture) SDL_DestroyTexture(gOpponentHandTexture);

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
				SendEndCommand();
				break;
			case SDL_MOUSEBUTTONUP:
				mouse = (SDL_MouseButtonEvent*)&event;
				if(mouse->button == SDL_BUTTON_LEFT){
					
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
                    /* じゃんけんボタンが押されたら表示をクリア */
                    if (buttonNO >= 0 && buttonNO <= 2) {
                        ClearResultImages(); // 古いじゃんけん画像をクリア
                        gOpponentHand = 0; // 相手の手をクリア
                    }

					switch(buttonNO) {
                        case 0: // グー
                            gMyHand = JANKEN_ROCK_COMMAND;
                            SendJankenCommand(gMyHand);
                            gWaitingForResult = 1; 
                            break;
                        case 1: // チョキ
                            gMyHand = JANKEN_SCISSORS_COMMAND;
                            SendJankenCommand(gMyHand);
                            gWaitingForResult = 1; 
                            break;
                        case 2: // パー
                            gMyHand = JANKEN_PAPER_COMMAND;
                            SendJankenCommand(gMyHand);
                            gWaitingForResult = 1; 
                            break;
                        case 3: // END
                            gMyHand = 0; // 自分の手をクリア
                            SendEndCommand();
                            break;
                    }

                    if (buttonNO >= 0 && buttonNO <= 2) {
                        /* 自分の手札をテキストで表示し、「Waiting」を出す */
                        DrawGameStatus("Waiting for opponent...", gMyHand, 0); // 相手の手はまだ不明
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
          char  opponentHand: 相手の手 (R, S, P)
出力	: なし
*****************************************************************/
void DrawResult(char result, char opponentHand)
{
    gWaitingForResult = 0; // 結果が来たので待ち状態解除
    gOpponentHand = opponentHand; // 相手の手を保存

    const char* resultText;

    switch(result) {
        case RESULT_WIN_COMMAND:
            resultText = "You WIN!"; // "Choose next hand." は DrawGameStatus で加える
            break;
        case RESULT_LOSE_COMMAND:
            resultText = "You LOSE...";
            break;
        case RESULT_DRAW_COMMAND:
            resultText = "DRAW.";
            break;
        default:
            resultText = "Error: Unknown result.";
            break;
    }

    /* 全てのステータスを描画 */
    DrawGameStatus(resultText, gMyHand, gOpponentHand);
}



/*****************************************************************
関数名	: HandToFileName
機能	: 手の文字 (R,S,P) をファイル名 ("R.png" 等) に変換する
引数	: char hand
出力	: const char*
*****************************************************************/
static const char* HandToFileName(char hand) {
    switch(hand) {
        case JANKEN_ROCK_COMMAND:    return "R.png";
        case JANKEN_SCISSORS_COMMAND:  return "S.png";
        case JANKEN_PAPER_COMMAND:    return "P.png";
        default:                    return NULL; // 無効な手
    }
}

/*****************************************************************
関数名	: LoadHandTexture
機能	: じゃんけんの手の画像テクスチャをロードする
引数	: char handChar : 手の文字 (R, S, P)
出力	: SDL_Texture* : ロードされたテクスチャ、失敗時はNULL
*****************************************************************/
static SDL_Texture* LoadHandTexture(char handChar) {
    const char* filename = HandToFileName(handChar);
    if (!filename) {
        return NULL;
    }

    SDL_Surface* image = IMG_Load(filename);
    if (!image) {
        printf("failed to load image: %s (SDL_image Error: %s)\n", filename, IMG_GetError());
        return NULL;
    }
    SDL_Texture* texture = SDL_CreateTextureFromSurface(gMainRenderer, image);
    SDL_FreeSurface(image);
    return texture;
}

/*****************************************************************
関数名	: ClearResultImages
機能	: 以前のじゃんけん手札画像を解放し、レンダラーからクリアする
引数	: なし
出力	: なし
*****************************************************************/
static void ClearResultImages(void) {
    // 既存のテクスチャを解放
    if (gMyHandTexture) {
        SDL_DestroyTexture(gMyHandTexture);
        gMyHandTexture = NULL;
    }
    if (gOpponentHandTexture) {
        SDL_DestroyTexture(gOpponentHandTexture);
        gOpponentHandTexture = NULL;
    }
    // 表示領域を白でクリア
    SDL_Rect clearRect = {550, 200, 450, 250}; // 右下部分の画像表示領域
	SDL_SetRenderDrawColor(gMainRenderer, 255, 255, 255, 255);
  	SDL_RenderFillRect(gMainRenderer, &clearRect);
    SDL_RenderPresent(gMainRenderer);
}

/*****************************************************************
関数名	: DrawGameStatus
機能	: ウィンドウ下部に全てのステータスを描画する
引数	: const char* resultMsg : 勝敗結果メッセージ
          char myHandChar    : 自分の手 (R, S, P, または 0)
          char oppHandChar   : 相手の手 (R, S, P, または 0)
出力	: なし
*****************************************************************/
static void DrawGameStatus(const char* resultMsg, char myHandChar, char oppHandChar)
{
    /* クリア領域を拡大 (Y=200から下) */
    SDL_Rect clearRect = {0, 200, 1020, 250}; 
	SDL_SetRenderDrawColor(gMainRenderer, 255, 255, 255, 255);
  	SDL_RenderFillRect(gMainRenderer, &clearRect);

    /* 左下に結果メッセージを表示 */
    stringColor(gMainRenderer, 30, 300, resultMsg, 0xff000000);
    
    // 次のじゃんけんを促すメッセージは、結果が出た場合にのみ表示
    if (strcmp(resultMsg, "Welcome! Choose your hand.") != 0 &&
        strcmp(resultMsg, "Waiting for opponent...") != 0 &&
        strcmp(resultMsg, "Error: Unknown result.") != 0) {
        stringColor(gMainRenderer, 30, 350, "Choose next hand.", 0xff0000ff);
    }

    /* 右側にじゃんけん手札の画像を表示 */
    // 自分の手
    if (myHandChar != 0) {
        if (gMyHandTexture) SDL_DestroyTexture(gMyHandTexture); // 古いテクスチャ解放
        gMyHandTexture = LoadHandTexture(myHandChar);
        if (gMyHandTexture) {
            stringColor(gMainRenderer, gMyHandImageRect.x, gMyHandImageRect.y - 30, "You:", 0xff000000);
            SDL_RenderCopy(gMainRenderer, gMyHandTexture, NULL, &gMyHandImageRect);
        }
    }

    // 相手の手
    if (oppHandChar != 0) {
        if (gOpponentHandTexture) SDL_DestroyTexture(gOpponentHandTexture); // 古いテクスチャ解放
        gOpponentHandTexture = LoadHandTexture(oppHandChar);
        if (gOpponentHandTexture) {
            stringColor(gMainRenderer, gOpponentHandImageRect.x, gOpponentHandImageRect.y - 30, "Opponent:", 0xff000000);
            SDL_RenderCopy(gMainRenderer, gOpponentHandTexture, NULL, &gOpponentHandImageRect);
        }
    }
    
    SDL_RenderPresent(gMainRenderer);
}


/*****************************************************************
関数名	: CheckButtonNO
機能	: クリックされたボタンの番号を返す
(中略)
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