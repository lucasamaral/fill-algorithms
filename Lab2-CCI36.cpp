// Lab2-CCI36.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"


int _tmain(int argc, _TCHAR* argv[])
{
	return 0;
}

#include "stdafx.h"


#include <stdlib.h>
#include <windows.h>
#include "graphbase.h"
#include <stdio.h>
#include <string.h>
#include <math.h>


#define PI 3.1415926535897932384626433832795
#define ENTER 13
#define BACKSPACE 8
#define ESC 27
#define NO_ACTION -1
#define L_MOUSE_MOVE_DOWN 0
#define L_MOUSE_DOWN 1
#define R_MOUSE_DOWN 2
#define L_DOUBLE_CLICK 3
#define L_MOUSE_UP 4
#define LINHA 1
#define TRACEJADO 2
#define PONTILHADO 3


#define CONSOLE_SIZE_X 640  // initial input line size in pixel
#define START_TEXT_X 5   // input line box initial position X
#define END_TEXT_X (CONSOLE_SIZE_X-2) // size of input line box in X
static SHORT numXpixels = 500;//640;			// size of window in X
static SHORT numYpixels = 500;//480             // size of window in Y
int start_text_y=numYpixels-20; 
int end_text_y = 21+start_text_y;					// size of input line in Y

int key_input=NO_ACTION,mouse_action=NO_ACTION;
int mouse_x, mouse_y;
static BOOL graphics = TRUE;                /* Boolean, enable graphics?  */


//static short draw_color = MY_WHITE;        /* Current drawing color.     */
char buffer[200]="";					// string buffer for keyboard input

HMENU menu,menu_draw, menu_color;

static COLORREF win_draw_color =RGB(255,255,255);  // current draw color
static HBRUSH blackBrush;
HDC  hdc;                                   /* Presentation Space Handle */
HWND WinHandle = NULL;                /* Client area window handle */
/* Foreground colors default to standard EGA palette.                */
/* No map is necessary unless a non-standard palette is implemented. */
static  HPEN hpen;
// 16 Predefined colors
typedef enum { MY_BLACK,MY_BLUE,MY_GREEN,MY_CYAN,MY_RED,MY_MAGENTA,
	MY_BROWN,MY_LIGHTGRAY,MY_DARKGRAY,MY_LIGHTBLUE,MY_LIGHTGREEN,
	MY_LIGHTCYAN,MY_LIGHTRED,MY_LIGHTMAGENTA,MY_YELLOW,MY_WHITE} my_color;
/* Define RGB color settings for MY enumerated colors */
static COLORREF color_trans_map[] =
{
	RGB(0,0,0),//MY_BLACK 
	RGB(0,0,255),//MY_BLUE,
	RGB(0,127,0),//MY_GREEN,
	RGB(0,233,233),//MY_CYAN,
	RGB(255,0,0),//MY_RED,
	RGB(255,0,255),//MY_MAGENTA,
	RGB(153,51,0),//MY_BROWN,
	RGB(175,175,175),//MY_LIGHTGRAY,
	RGB(70,70,70),//MY_DARKGRAY,
	RGB(51,51,255),//MY_LIGHTBLUE,
	RGB(0,255,0),//MY_LIGHTGREEN,
	RGB(51,255,255),//MY_LIGHTCYAN,
	RGB(255,25,25),//MY_LIGHTRED,
	RGB(255,65,255),//MY_LIGHTMAGENTA,
	RGB(255,255,0),//MY_YELLOW,
	RGB(255,255,255),//MY_WHITE,
};

struct Vertex{ //os v�rtices ser�o adicionados em uma lista ligada
	int x;
	int y;
	Vertex *next;
};

Vertex *createVertex(int x, int y){//fun��o para a cria��o de um v�rtice
	Vertex* v = (Vertex*) malloc(sizeof(Vertex));
	v->x = x;
	v->y = y;
	v->next = NULL;
	return v;
}

struct Edge{ //estrutura de uma aresta
	int Ymax, Ymin; //Y m�ximo e Y m�nimo
	float Xinter, dx; //x intermedi�rio e o passo dx
};

struct ListEdge{//estrtura do vetor que cont�m as arestas
	int numArestas; //n�mero de arestas do vetor
	Edge* edges; //vetor com as arestas
};

ListEdge *createListEdge(int numArestas){//inicializa��o da lista de arestas
	ListEdge* le = (ListEdge*) malloc(sizeof(ListEdge)); //aloca��o da estrutura
	le->numArestas = numArestas; //definindo o n�mero de arestas
	le->edges = (Edge*) malloc(numArestas * sizeof(Edge));
	for (int i = 0; i < numArestas; i++) //incializando todos os valores das arestas
	{
		le->edges[i].dx = 0;
		le->edges[i].Xinter = 0;
		le->edges[i].Ymax = 0;
		le->edges[i].Ymin = 0;
	}
	return le;
}

struct Poligono{// estrutura do pol�gono
	int numLados; //n�mero de lados
	Vertex* primeiro; //ponteiro para o primeiro v�rtice da lista ligada 
};

Poligono *initPoligono(){//incializa o pol�gono
	Poligono* p = (Poligono*) malloc(sizeof(Poligono));
	p->numLados = 0;
	p->primeiro = NULL;
	return p;
}

void GetPoint(Poligono *p, int k, int* x, int* y) {
	Vertex *head = p->primeiro;
	for (int i = 0; i < k; i++)
	{
		head = head->next;
	}
	*x = head->x;
	*y = head->y;
}

void PolyInsert(ListEdge* list,int x1,int y1,int x2,int y2)
{     // insert line segment in edge struct, if not horizontal
	if (y1!=y2)
	{
		int YM=max(y1,y2),J1=list->numArestas +1;
		while (J1!=1 &&  list->edges[J1-1].Ymax <YM)
		{
			list->edges[J1]=list->edges[J1-1];
			J1--;
		}

		list->edges[J1].Ymax=YM;
		list->edges[J1].dx = -1*(float)(x2-x1)/(y2-y1);
		if (y1>y2)
		{
			list->edges[J1].Ymin=y2;
			list->edges[J1].Xinter=(float)x1;
		}
		else {
			list->edges[J1].Ymin=y1;
			list->edges[J1].Xinter=(float)x2;
		}
		list->numArestas++;
	}
}

void LoadPolygon(Poligono* p, ListEdge* list, int* num_Edges)
{
	int x1,x2,y1,y2,k=1;

	list->numArestas = 0;
	GetPoint(p, k, &x1, &y1);
	*num_Edges = 0;

	for ( ; k <= p->numLados ; k++ ) 
	{
		GetPoint(p, k%p->numLados+1,&x2,&y2);
		if (y1==y2) x1 = x2 ;
		else 
		{
			PolyInsert(list, x1,y1,x2,y2);
			*num_Edges+=1;
			x1=x2;
			y1=y2;
		}
	}
} 

void Include(ListEdge* list, int* end_Edge, int* final_Edge, int* scan)
{	// include all edges that intersept y_scan
   while (*end_Edge < *final_Edge && list->edges[*end_Edge+1].Ymax >= *scan)
   {
	(*end_Edge) = (*end_Edge)+1;
   }
}

void XSort( ListEdge* list, int start_Edge, int last_Edge)
{  
	int L,k ; 
	bool sorted=false;
	Edge* temp;
	// Use bubble sort

	for ( k = start_Edge ; k < last_Edge; k++ )
	{
		L = k + 1; 
		while ( L > start_Edge && 
			list->edges[L].Xinter < list->edges[L-1].Xinter )
		{
			temp=&list->edges[L];
			list->edges[L]=list->edges[L-1];
			list->edges[L-1]=*temp;
			L--;
		}
	}
} 

void FillIn ( int x1 ,int x2 ,int  y )
{
	int px,py;
	py = y  % 8 ;
	if (x1!=x2)	{
		for (int x = x1 ; x <= x2 ; x++ ) 
		{
			px=x%8;
			DrawPixel(x,y);
		}
		
	} 
}

void FillScan (ListEdge* list, int end_Edge , int start_Edge ,int scan )
{
	int NX , J , K;

	NX = (end_Edge - start_Edge + 1)/2;
	J = start_Edge;

	for ( K = 1 ; K <= NX ; K++ )
	{
		/*if((int)list->edges[J].Xinter != (int)list->edges[J+1].Xinter)*/
			FillIn ( (int)list->edges[J].Xinter, (int)list->edges[J+1].Xinter, scan);
		/*else
			FillIn ( (int)list->edges[J].Xinter, (int)list->edges[J+2].Xinter, scan);
		J += 2;*/
	}
}

void UpdateXValues(ListEdge* list, int last_Edge ,  int* start_Edge ,int* scan)
{
	int K1;

	for ( K1 = *start_Edge ; K1 <= last_Edge ; K1++)
	{
		if ( list->edges[K1].Ymin < *scan )
		{
			list->edges[K1].Xinter += list->edges[K1].dx ;
		}
		else 
		{  // remove edge
			(*start_Edge) = (*start_Edge)+1;
			if ( *start_Edge <= K1 ) 
				for ( int i = K1 ; i >= *start_Edge ; i--)
					list->edges[i]=list->edges[i-1]; 
		}
   }
}
	
void FillPolygon ( Poligono* p,  ListEdge* list )
{
	int Edges, start_Edge , end_Edge , scan;

	LoadPolygon(p,list, &Edges);

	if (Edges==2) return;
	scan = list->edges[1].Ymax ;
	start_Edge = 1;
	end_Edge = 0;

	Include(list, &end_Edge, &Edges, &scan);

	while ( end_Edge != start_Edge - 1 ) 
	{
		XSort(list, start_Edge, end_Edge);
		FillScan(list, end_Edge, start_Edge, scan);
		scan--;
		UpdateXValues(list, end_Edge, &start_Edge, &scan);
		Include(list, &end_Edge, &Edges, &scan);
	}
}

/****************************************************************************
*                           The  Circle Fill                                *
*                              EVERYTHING                                   *
****************************************************************************/

struct Circle
{
	int x_center;
	int y_center;
	int radius;
};

Circle* createCircle(int x, int y, int r)
{
	Circle* circle = (Circle*) malloc(sizeof(Circle));
	circle->x_center = x;
	circle->y_center = y;
	circle->radius = r;
	return circle;
}

struct Stack {
	Vertex *head;
};

Stack* createQueue()
{
	Stack *pilha = (Stack*) malloc(sizeof(Stack));
	pilha->head = NULL;
	return pilha;
}

void emPilha(Stack *pilha, Vertex *vert)
{
	Vertex* first = pilha->head;
	pilha->head = vert;
	vert->next = first;
}

Vertex* desemPilha(Stack *pilha)
{
	Vertex *first = pilha->head;
	pilha->head = pilha->head->next;
	return first;
}


void floodFillCircle(int x, int y, COLORREF fcolor,COLORREF bcolor)
{
	COLORREF current;
	Stack *queue = createQueue();
	Vertex *node;
	current= GetPixel(x,y);
	if(current != color_trans_map[bcolor] && current != color_trans_map[fcolor])
		node = createVertex(x,y);
	else
		return;
	emPilha(queue,node);
	Vertex* n = desemPilha(queue);
	boolean first = true;
	while(queue->head != NULL || first)
	{
		if(first)
			first = false;
		current= GetPixel(n->x,n->y);
		if(current != color_trans_map[bcolor] && current != color_trans_map[fcolor])
		{
			SetGraphicsColor(fcolor,1);
			DrawPixel(n->x,n->y);
			Vertex* oeste = createVertex(n->x-1,n->y);
			Vertex* leste = createVertex(n->x+1,n->y);
			Vertex* norte = createVertex(n->x,n->y-1);
			Vertex* sul = createVertex(n->x,n->y+1);
			free(n);
			emPilha(queue,oeste);
			emPilha(queue,leste);
			emPilha(queue,norte);
			emPilha(queue,sul);
		}
		n = desemPilha(queue);
	}
}

void bfill8(int x,int y,COLORREF fcolor,COLORREF bcolor)
{
	COLORREF current;
	current= GetPixel(x,y);
	if(current != color_trans_map[bcolor] && current != color_trans_map[fcolor])
	{
		SetGraphicsColor(fcolor,1);
		DrawPixel(x,y);
		//Sleep(1);
		bfill8(x,y+1,fcolor,bcolor);
		bfill8(x,y-1,fcolor,bcolor);
		bfill8(x+1,y-1,fcolor,bcolor);
		bfill8(x+1,y+1,fcolor,bcolor);
		bfill8(x-1,y-1,fcolor,bcolor);
		bfill8(x-1,y+1,fcolor,bcolor);
		bfill8(x+1,y,fcolor,bcolor);
		bfill8(x-1,y,fcolor,bcolor);
	}
}


/****************************************************************************
*  Set the X dimension of the current window in pixels.                 *
****************************************************************************/
void SetMaxX(int maxX)
{
	numXpixels=maxX;
}
/****************************************************************************
*  Set the X dimension of the current window in pixels.                 *
****************************************************************************/
void SetMaxY(int maxY)
{
	numYpixels = maxY;
	start_text_y=numYpixels-20; 
	end_text_y = 21+start_text_y;
}

void DrawXorPixel(int x, int y)
{ 
	unsigned int mask=0x00FFFFFF;

	COLORREF cor=GetPixel(hdc,x,y);

	cor^=mask; // bit-bit Xor operation mask with color
	SetPixel(hdc,x,y,cor);
}

/****************************************************************************
*  Draws a pixel at the specified point on the screen.                      *
*  Caution!! GpiSetPel has been found to crash programs at some locations!  *
****************************************************************************/
int count = 0;

void DrawPixel(int x, int y)
{
	SetPixel(hdc, x,y,win_draw_color);
}


/****************************************************************************
*                             Menu Creation                                 *
****************************************************************************/

void MenuBar()
{
	menu=CreateMenu();
	menu_draw=CreatePopupMenu();
	menu_color=CreatePopupMenu();

	AppendMenu(
		menu,      // handle to menu to be changed
		MF_POPUP,      // menu-item flags
		(UINT)menu_draw,  // menu-item identifier or handle to drop-down menu or submenu
		(LPCTSTR)L"&Draw" // menu-item content
		);


	InsertMenu(menu_draw,0,  MF_STRING, 21,  (LPCTSTR)L"&Line" );

	AppendMenu(menu_draw,  MF_STRING, 22,  (LPCTSTR)L"&Circle" );

	AppendMenu(menu,   MF_POPUP,      (UINT)menu_color, (LPCTSTR)L"&Color");

	InsertMenu(menu_color,0,  MF_STRING, 1,  (LPCTSTR)L"Black" );
	AppendMenu(menu_color,  MF_STRING, 2,  (LPCTSTR)L"Blue" );
	AppendMenu(menu_color,  MF_STRING, 3,  (LPCTSTR)L"Green" );
	AppendMenu(menu_color,   MF_STRING,     4, (LPCTSTR)L"Cyan");
	AppendMenu(menu_color,   MF_STRING,      5, (LPCTSTR)L"Red");

	AppendMenu(menu_color,  MF_STRING, 6,  (LPCTSTR)L"Magenta" );
	AppendMenu(menu_color,  MF_STRING, 7,  (LPCTSTR)L"Brown" );
	AppendMenu(menu_color,   MF_STRING,     8, (LPCTSTR)L"LightGray");
	AppendMenu(menu_color,   MF_STRING,     9, (LPCTSTR)L"DarkGray");

	AppendMenu(menu_color,  MF_STRING, 10,  (LPCTSTR)L"LightBlue" );
	AppendMenu(menu_color,  MF_STRING, 11,  (LPCTSTR)L"LightGreen" );
	AppendMenu(menu_color,  MF_STRING, 12,  (LPCTSTR)L"LightCyan" );
	AppendMenu(menu_color,   MF_STRING,     13, (LPCTSTR)L"LightRed");
	AppendMenu(menu_color,   MF_STRING,     14, (LPCTSTR)L"LightMagenta");
	AppendMenu(menu_color,   MF_STRING,     15, (LPCTSTR)L"Yellow");
	AppendMenu(menu_color,   MF_STRING,     16, (LPCTSTR)L"White");
}

/****************************************************************************
*                                                                           *
*  Name       :  InitGraphics()                                             *
*                                                                           *
*  Description:   Initializes the process for Window services               *
*  Concepts   : - obtains anchor block handle							    *
*  - creates the main frame window which creates the                        *
*  main client window                                                       *
*                                                                           *                                                                           *
*                                                                           *
****************************************************************************/

wchar_t wind_class[]=L"Window Application";
wchar_t wind_name[]= L"Lab2 CCI36";
void InitGraphics()
{

	HINSTANCE hInst=NULL;
	HWND hWnd; 
	WNDCLASS    wc;
	LPCWSTR window_class=(LPCWSTR) wind_class;
	//
	LPCWSTR window_name=(LPCWSTR) wind_name;
	// Fill up window structure 

	wc.lpszClassName =  window_class;  // registration name
	wc.hInstance = hInst;				// application instance
	wc.lpfnWndProc =  (WNDPROC) WinProc; // event handling function
	wc.hCursor = LoadCursor(NULL, IDC_ARROW); // cursor type
	wc.hIcon = NULL;
	wc.lpszMenuName = NULL;						// menu, if any
	wc.hbrBackground = (HBRUSH)GetStockObject(BLACK_BRUSH); // background color
	wc.style = CS_HREDRAW|CS_VREDRAW;		// window style
	wc.cbClsExtra = 0;
	wc.cbWndExtra = 0;

	MenuBar();
	/* Registeer window class      */
	//GlobalAddAtom(window_class);

	if(!RegisterClass(&wc))
	{
		printf(" Error in RegisterClass...\n");
		exit(1);
	}

	// Create window
	hWnd = CreateWindow(
		window_class,           // Desktop window class name             
		window_name,       // window name                 
		WS_OVERLAPPEDWINDOW | WS_VISIBLE,  // Window class style                  
		0, 0,                //window  top, left corner(origin)
		500, 500 ,                   // window X,Y size                                    
		(HWND)NULL,                   // Parent window         /
		(HMENU)menu,				// handle to menu 
		(HINSTANCE) hInst,			// handle to application instance 
		(LPVOID)NULL);  //  pointer to window-creation data  

	if ((hWnd == NULL))
	{

		printf("error in CreateWindow ...\n ");
		exit(1);

	}

	// Sets the visibility state of window 
	ShowWindow(hWnd,SW_SHOW);

	// store window handle device context 
	WinHandle = hWnd;
	hdc = GetDC(WinHandle);
	// set hpen, blackbrush for clearing window, color for text and text background
	hpen = CreatePen(PS_SOLID, 1, win_draw_color);
	SelectObject(hdc, hpen);
	blackBrush = (HBRUSH) GetStockObject(BLACK_BRUSH);
	SetBkColor(hdc,RGB(0,0,0));
	SetTextColor(hdc,RGB(255,255,255));

}


/****************************************************************************
*  Reset display to default mode.                                           *
****************************************************************************/
void CloseGraphics(void)
{

	// Delete pen and destroy window
	DeleteObject(hpen);
	ReleaseDC(WinHandle,hdc);
	DestroyWindow(WinHandle);          /* Tidy up...                 */
}


/****************************************************************************
*  Returns the X dimension of the current window in pixels.                 *
****************************************************************************/
int GetMaxX(void)
{
	return numXpixels;
}

/****************************************************************************
*  Returns the Y dimension of the current window in pixels.                 *
****************************************************************************/
int GetMaxY(void)
{
	return numYpixels;
}


/****************************************************************************
*  Set current graphics drawing color.                                      *
****************************************************************************/
void SetGraphicsColor(int new_color, int width)
{  
	HPEN hpenOld;
	if (graphics)
	{  
		// test to avoid unnecessay pen changing
		if (win_draw_color!=color_trans_map[new_color])
		{
			// get COLORREF from defined palette
			win_draw_color = color_trans_map[new_color];

			// create a pen with that color 
			hpen = CreatePen(PS_SOLID, width, win_draw_color);
			hpenOld = (HPEN)SelectObject(hdc, hpen);
			//   delete old pen
			DeleteObject(hpenOld);
		}
	}

}


/****************************************************************************
*  Returns the color value of the pixel at the specified point on the       *
*  screen.                                                                  *
****************************************************************************/
int GetPixel(int x, int y)
{
	return (int)GetPixel(hdc, x,y);
}

void CheckGraphicsMsg(void)
{  MSG msg;
/*Peek Message from message queue */
if (PeekMessage(&msg, WinHandle, 0L, 0L, PM_REMOVE))
	//  Translate keyboard hit to virtual key code and send to message queue
{   TranslateMessage(&msg);
DispatchMessage(&msg);
}

}
void ClearString(char *str)
{ str[0]=0x00;
}
void EraseMessage()
{
	RECT rec={START_TEXT_X,start_text_y,
		END_TEXT_X,end_text_y}; 
	HBRUSH backgrdBrush= ( HBRUSH )GetStockObject(BLACK_BRUSH);
	// Clear input input box
	FillRect(hdc,&rec,backgrdBrush);
}
void  PrintMessage(char *buffer)
{
	// Write input text in the graphics window
	// Input line is in the upper portion of the graphics window
	if (strlen(buffer)>0)
		TextOutA(hdc,START_TEXT_X,start_text_y,(LPCSTR)buffer,strlen(buffer));

}

int menu_item;

/****************************************************************************
*  Mouse Handler for Win 95                                                   *
****************************************************************************/
static LRESULT CALLBACK WinProc(HWND hWnd,UINT messg,WPARAM wParam,LPARAM lParam)
{

	PAINTSTRUCT ps;

	char str[3]=" ";
	switch (messg)
	{

	case WM_PAINT:
		BeginPaint(hWnd,&ps);
		PrintMessage(buffer);
		// Draw everything
		//	  RefreshScreen();

		ValidateRect(hWnd,NULL);

		EndPaint(hWnd,&ps);
		break;
	case WM_CHAR:
		{
			// Win32 keyboard message: lParam=key code wParam= virtual key (ASCII) code 

			if (!(LOWORD(wParam) & KF_UP) &&
				!(LOWORD(wParam) & KF_ALTDOWN )    )

			{
				//  take keyboard input
				key_input = (char)LOWORD(wParam);



				if (key_input==ENTER) // Enter
				{
					EraseMessage();
					//buffer[strlen(buffer)]=0x00;
				}
				else if (key_input==BACKSPACE) // BackSpace
				{   if (strlen(buffer)>0)
				{ int len=strlen(buffer)-1;
				// Clear last character in buffer
				buffer[len]=' ';
				// Clear characters in input box
				strcat_s(buffer,"   ");
				PrintMessage(buffer);
				buffer[len]=0x00; // put end string
				}
				}
				else if (key_input>31 && key_input<130)
				{ int leng=strlen(buffer);

				str[0]=key_input;
				strcat_s(buffer,str); // add char
				// display, update input box
				PrintMessage(buffer);		
				}
				else if (key_input!=ESC) //ESC
					key_input = -1;
				break;
			}
		}
	case WM_SIZE:
		// resize 
		SetMaxX(LOWORD(lParam));  // width of client area 
		SetMaxY(HIWORD(lParam));            
		PostMessage(WinHandle, WM_PAINT, wParam, lParam);

		break;
	case WM_MOUSEMOVE:

		{     key_input = wParam;   
		if (key_input==MK_LBUTTON)

		{
			EraseMessage();
			mouse_x = LOWORD(lParam);
			mouse_y = HIWORD(lParam);
			key_input = wParam;  
			printf_s(buffer," x = %d y = %d",mouse_x,mouse_y);
			PrintMessage(buffer);
			mouse_action =  L_MOUSE_MOVE_DOWN;  
			ClearString(buffer);
		}
		break;
		}
	case WM_LBUTTONDOWN:

		{

			EraseMessage();
			mouse_x = LOWORD(lParam);
			mouse_y = HIWORD(lParam);
			key_input = wParam;  
			printf_s(buffer," x = %d y = %d",mouse_x,mouse_y);
			PrintMessage(buffer);
			mouse_action =  L_MOUSE_DOWN;  
			ClearString(buffer);
			break;
		}
	case WM_LBUTTONUP:

		{

			EraseMessage();
			mouse_x = LOWORD(lParam);
			mouse_y = HIWORD(lParam);
			key_input = wParam;  
			sprintf_s(buffer," x = %d y = %d",mouse_x,mouse_y);
			PrintMessage(buffer);
			mouse_action =  L_MOUSE_UP;  
			ClearString(buffer);
			break;
		}

	case WM_RBUTTONDOWN:
		{    	   
			EraseMessage();
			key_input = wParam;        
			mouse_x = LOWORD(lParam);
			mouse_y = HIWORD(lParam);

			sprintf_s(buffer," x = %d y = %d",mouse_x,mouse_y);
			PrintMessage(buffer);

			mouse_action =  R_MOUSE_DOWN; 
			ClearString(buffer);

			break;
		}
	case WM_LBUTTONDBLCLK: 
		EraseMessage();
		mouse_x = LOWORD(lParam);
		mouse_y = HIWORD(lParam);
		key_input = wParam;  
		sprintf_s(buffer," x = %d y = %d",mouse_x,mouse_y);
		PrintMessage(buffer);
		mouse_action =  L_DOUBLE_CLICK; 
		ClearString(buffer);
		key_input = wParam;  

		break; 
	case WM_DESTROY:
		PostQuitMessage(0);
		break;
	case WM_COMMAND:
		menu_item=LOWORD(wParam);
		break;
	default: 
		return(DefWindowProc(hWnd,messg,wParam,lParam));
		break;
	}

	return 0;

}

/****************************************************************************
*  Draws a line between two specified points on the screen.                 *
****************************************************************************/
void DrawLine(int x, int y, int x2, int y2)
{
   
    if (graphics)
    {
        MoveToEx(hdc,x,y,NULL);    
        LineTo(hdc, x2,y2);	 
    }
}

/****************************************************************************
*  Draw a ellipse on the screen.                                            *
****************************************************************************/

void DrawEllipse(int x, int y, int x_radius, int y_radius, int filled)
{    
    int x1,y1,x2,y2;
//			Win32 Ellipse function requires ellipse bounding box as input
    if (graphics)
    {
		// calculate the bounding box
		if (x_radius>0)
		{
          x1=x-x_radius;  
	      x2=x+x_radius;
		}
		else if (x_radius==0)// make the ellipse 2 pixels wide (a line)
		{ x1=x;
		  x2=x+1;
		}
		 else return; // wrong radius
		if (y_radius>0)
		{
          y1=y-y_radius;  
	      y2=y+y_radius;
		}
		else if (y_radius==0) // make the ellipse 2 pixels wide (a line)
		{ y1=y;
		  y2=y+1;
		}
		 else return;
      
	 
    
        if (!filled)
        {	// Need to select NULL_BRUSH to avoid filling the ellipse
			HBRUSH brush =(HBRUSH)GetStockObject(NULL_BRUSH);
		    SelectObject(hdc,brush);
            Ellipse(hdc,x1,y1,x2,y2);  // Draw ellipse
        }
        else
		
        {			// Draw a solid ellipse with a brush of current color 
			HBRUSH brush = CreateSolidBrush(win_draw_color);
		    SelectObject(hdc,brush);
			Ellipse(hdc,x1,y1,x2,y2);
					// remove brush
			DeleteObject(brush);
        }
		   
	

    }
}

void DrawCircle(int x, int y, int r)
{
	DrawEllipse(x,y,r,r,0);
}


int Max(int x, int y)
{  
	return (x>y)?x:y;
}

#define Arred(x) ((int)(x+0.5)) //soma 0.5 para que seja feito o arredondamento da maneira desejada

void DrawLine(int x1, int y1, int x2, int y2, int tipo)
{  // i,length s�o inteiros   x,y,dx,dy s�o reais
	int cont = 0;
	int i, length;
	float x,y,dx,dy;
	length=Max(abs(x2-x1),abs(y2-y1)); //o comprimento ser� o maior valor entre varia��oX e varia��oY

	if (length>0)
	{
		dx=((float)(x2-x1))/length; //o passo em X � calculado por esse quociente
		dy=((float)(y2-y1))/length; //an�logo a X
		x=(float)x1; y=(float)y1; //ponto inicial � x1,y1
		for (i=0; i<=length; i++) //desde o comprimento 0 at� o comprimento m�ximo ser�o desenhados os pixels
		{
			cont++;
			switch (tipo) //o par�metro tipo ir� definir qual o tipo de tra�o do desenho
			{
			case LINHA:
				DrawPixel(Arred((int)x),Arred((int)y)); break;
			case PONTILHADO: 
				if(cont%2!=0) DrawPixel(Arred((int)x),Arred((int)y)); break;
			case TRACEJADO:
				if(cont%8!=0) DrawPixel(Arred((int)x),Arred((int)y)); break;
			default:
				break;
			}
			x=x+ dx;    // incremento em x
			y=y+ dy;   // incremento em y
		}
	}
}

void DrawXorLine(int x1, int y1, int x2, int y2, int tipo)
{  //A XorLine utiliza DrawXorPixel
	int cont = 0;
	int i, length;
	float x,y,dx,dy;
	length=Max(abs(x2-x1),abs(y2-y1));

	if (length>0)
	{
		dx=((float)(x2-x1))/length;
		dy=((float)(y2-y1))/length;
		x=(float)x1; y=(float)y1;
		for (i=0; i<=length; i++)
		{
			cont++;
			switch (tipo)
			{
			case LINHA:
				DrawXorPixel(Arred((int)x),Arred((int)y));
				break;
			case PONTILHADO: 
				if(cont%2!=0)
					DrawXorPixel(Arred((int)x),Arred((int)y));
				break;
			case TRACEJADO:
				if(cont%8!=0)
					DrawXorPixel(Arred((int)x),Arred((int)y));
				break;
			default:
				break;
			}
			x=x+ dx;    // dx = 1 ou -1 ou m
			y=y+ dy;   // yx = 1 ou -1 ou 1/m
		}
	}
}

void swap(int* a, int* b) //fun��o para trocar os valores das vari�veis a e b
{
	int temp = *a;
	*a = *b;
	*b = temp;
}

void DrawLineBresenham(int x1, int y1, int x2, int y2, int tipo)
{
	int cont = 0; //contador para o tipo de linha a ser desenhada
	boolean steep = abs(y2 - y1) > abs(x2-x1); //vari�vel que indica se a reta � decrescente ou n�o
	if(steep) //quando a reta � decrescente troca as vari�veis para cair no caso do primeiro quadrante
	{
		swap(&x1,&y1); 
		swap(&x2,&y2);
	}
	int dx = abs(x2-x1); //varia��o em x
	int dy = abs(y2-y1); //varia��o em y
	int error = dx/2; //vari�vel que servir� para saber qual o pixel mais pr�ximo da reta
	int ystep; //passo em y
	int y = y1; //ponto atual
	int inc; //passo em x;

	inc = x1 < x2 ? 1 : -1; //se x1 for menor que x2 o passo � positivo caso contr�rio negativo
	ystep = y1 < y2 ? 1 : -1; //an�logo a "x" para "y"

	for(int x = x1; x != x2; x+=inc) //loop para percorrer a reta utilizando x como vari�vel base
	{
		cont++;
		if(steep) //se a reta � decrescente x e y foram trocados
		{
			switch (tipo)
			{
			case LINHA:
				DrawPixel(y,x); //na hora de imprimir destroca o x,y que tinham sido trocados
				break;
			case PONTILHADO: 
				if(cont%2!=0)
					DrawPixel(y,x);
				break;
			case TRACEJADO:
				if(cont%8!=0)
					DrawPixel(y,x);
				break;
			default:
				break;
			}
		}
		else  //reta crescente x e y est�o nas posi��es corretas
		{
			switch (tipo)
			{
			case LINHA:
				DrawPixel(x,y);
				break;
			case PONTILHADO: 
				if(cont%2!=0)
					DrawPixel(x,y);
				break;
			case TRACEJADO:
				if(cont%8!=0)
					DrawPixel(x,y);
				break;
			default:
				break;
			}
		}

		error -= dy;
		if(error < 0) //se error fica menor que 0 ent�o incrementar y pois o pr�ximo y � de um pixel mais pr�ximo da reta
		{
			y += ystep;
			error += dx;
		}
	}
}

void drawCircle8(int xc, int yc, int x, int y) //a partir de um ponto no primeiro quadrante s�o desenhados
{                                              //por simetria 8 outros pontos da circunfer�ncia
	DrawPixel(xc+x, yc+y);
	DrawPixel(xc-x, yc+y);
	DrawPixel(xc-x, yc-y);
	DrawPixel(xc+x, yc-y);
	DrawPixel(xc+y, yc+x);
	DrawPixel(xc-y, yc+x);
	DrawPixel(xc+y, yc-x);
	DrawPixel(xc-y, yc-x);
}

void DrawCircleBresenham(int xc, int yc, int R, int tipo)
{ //algoritimo de desenho da circunfer�ncia semelhante ao da reta de Bresenham
	//mas para cada ponto da circunfer�ncia s�o desenhados 8 outros pontos por simetria, economizando processamento
	int cont = 0;
	int x = R;
	int y = 0;
	int dx = 1-2*R;
	int dy = 1;
	int error = 0;

	while( x >= y)
	{
		cont++;
		switch (tipo)
		{
		case LINHA:
			drawCircle8(xc, yc, x, y);
			break;
		case PONTILHADO: 
			if(cont%2!=0)
				drawCircle8(xc, yc, x, y);
			break;
		case TRACEJADO:
			if(cont%8!=0)
				drawCircle8(xc, yc, x, y);
			break;
		default:
			break;
		}
		y++;
		error += dy;
		dy += 2;
		if(2*error + dx >0)
		{
			x--;
			error += dx;
			dx += 2;
		}
	}
}

void printPoligono(Poligono* p)
{
	Vertex* head = p->primeiro;
	printf("num lados: %d\n",p->numLados);
	int i = 0;
	while(head != NULL){
		printf("Ponto: %d - x= %4d  y=%4d\n", i, head->x,head->y);
		head = head->next;
		i++;
	}
}

void main()
{
	int p0_x, p0_y, p1_x,p1_y, menu_it=0, draw=1, color=MY_WHITE;
	InitGraphics();
	
	Poligono* p = initPoligono();
	Vertex* current = NULL;

	menu_item=0;
	CheckMenuItem(menu_color,1,MF_CHECKED);
	CheckMenuItem(menu_draw,21,MF_CHECKED);
	boolean firstPoint = true;
	while (key_input!=ESC)  // ESC exits the program
	{
		CheckGraphicsMsg();
		if (menu_it!=menu_item)
			switch(menu_item){
			case 21:{
				CheckMenuItem(menu_draw,22,MF_UNCHECKED); 
				CheckMenuItem(menu_draw,21,MF_CHECKED);
				menu_it=menu_item;
				draw=1;
				break;
					} 
			case 22:
				{
					CheckMenuItem(menu_draw,21,MF_UNCHECKED); 
					CheckMenuItem(menu_draw,22,MF_CHECKED);
					menu_it=menu_item;
					draw=2;
					break;
				}
			default:
				{ int i; 
				for (i=1; i<=16; i++)
					CheckMenuItem(menu_color,i,MF_UNCHECKED); 
				CheckMenuItem(menu_color,menu_item,MF_CHECKED);
				if (menu_item>=1 && menu_item<=16)
					color=menu_item-1;

				menu_it=menu_item;

				}
		}
		
		if (mouse_action==L_MOUSE_DOWN)
		{  // Pick first point up 
			if(firstPoint){
				p0_x=p1_x=mouse_x;
				p0_y=p1_y=mouse_y;
				Vertex* v = createVertex(p0_x,p0_y);
				p->primeiro = v;
				current = v;
			}
		}
		if (mouse_action==L_MOUSE_MOVE_DOWN)
		{  // Example of elastic line
			if (p1_x!=mouse_x || p1_y!=mouse_y)
			{  // Erase previous line. NOTE: It can improved using XOR line
				if(draw==1)
				{
					SetGraphicsColor((int)MY_BLACK,1);
					DrawLine(p0_x,p0_y,p1_x,p1_y);
					p1_x=mouse_x;
					p1_y=mouse_y;  // Draw new line
					SetGraphicsColor((int)MY_LIGHTGREEN,1);
					DrawLine(p0_x,p0_y,p1_x,p1_y);
				}
				else
				{
					SetGraphicsColor((int)MY_BLACK,1);
					DrawCircle(p0_x,p0_y,sqrt((float)pow((float)(p1_x-p0_x),2)+pow((float)(p1_y-p0_y),2)));
					p1_x=mouse_x;
					p1_y=mouse_y;
					SetGraphicsColor((int)MY_LIGHTGREEN,1);
					DrawCircle(p0_x,p0_y,sqrt((float)pow((float)(p1_x-p0_x),2)+pow((float)(p1_y-p0_y),2)));
				} 
			}	 
		}
		else  if(mouse_action==L_MOUSE_UP)
		{	
			SetGraphicsColor(color,2);
			if (draw==1){
				DrawLine(p0_x,p0_y,p1_x,p1_y);//desenha a aresta
				Vertex* final = createVertex(p1_x,p1_y);//criando e adicionando v�rtice ao final da lista
				current->next = final;
				current = current->next;//move para o pr�ximo v�rtice da lista ligada
				current->next = NULL;
				p->numLados++;//aumenta o tamanho de lados
				p0_x = p1_x;//para cada aresta desenhada o pr�ximo p0 � o p1 da aresta anterior.
				p0_y = p1_y;
				if(firstPoint)// testa se � o primeiro ponto desenhado
					firstPoint = false;
			}
			else
				DrawCircle(p0_x,p0_y,sqrt((float)pow((float)(p1_x-p0_x),2)+pow((float)(p1_y-p0_y),2)));	
			mouse_action=NO_ACTION;
		}
		else if (mouse_action==R_MOUSE_DOWN)// clique no bot�o direito do mouse ao terminar a figura
		{
			if(draw == 1)
			{
				Vertex* final = createVertex(p->primeiro->x,p->primeiro->y);
				current->next = final;
				DrawLine(p->primeiro->x,p->primeiro->y,current->x,current->y);
				p->numLados++;
				ListEdge *listaArestas = createListEdge(p->numLados);
				FillPolygon(p,listaArestas);
			}
			else
			{
				Circle* circle = createCircle(p0_x,p0_y,sqrt((float)pow((float)(p1_x-p0_x),2)+pow((float)(p1_y-p0_y),2)));
				floodFillCircle(circle->x_center, circle->y_center, MY_BLUE, MY_WHITE);
				//bfill8(circle->x_center, circle->y_center, MY_BLUE, MY_WHITE);
			} 
		}
	
	}
	CloseGraphics();
}
