#undef UNICODE
#undef _UNICODE
#include "Windows.h"
#include <string>
#include <vector>
using namespace std;

HWND hWnd;
HWND button;
HWND outputWindow;
HWND inputFrequency;
HWND stopAt;
HWND triggerButton;
HWND stopButton;
HWND helpButton;
HWND groupBox;
HWND toggle;
HWND press;
HWND statusText;
HWND rmlGroupBox;
HWND rightM;
HWND middleM;
HWND leftM;
HWND keyPressButton;

WNDCLASS windClass;
HINSTANCE hInstance;

bool quit=false;
bool switchFlag = true;
int numClicks = 0;
int numClicksSinceStop = 0;
bool doToggle = false;
unsigned char toggleState = 0;
bool waitingForTrigger = false;
int status = 0; //0 = idle, 1 = trigger key, 2 = clicking, 3 = press key
int prevStatus = 0;
bool clickedOnceForTriggerFlag = false;
bool clickedOnceForKeyFlag = false;
int mouseToClick = 0; //0=left 1=middle 2=right;
bool sameTriggerAndClick = false;
bool waitingForTriggerUp = false;

bool waitingForKey = false;

unsigned char keyDown[VK_OEM_CLEAR];
unsigned char keyPrev[VK_OEM_CLEAR];
unsigned char keyTrig[VK_OEM_CLEAR];
unsigned char keyUpTrig[VK_OEM_CLEAR];

#define TRIGGER_BTN 1000
#define STOP_BTN 2000
#define INPUT_TEXT 3000
#define OUTPUT_TEXT 4000
#define HELP_BTN 5000
#define KEY_PRESS_BTN 6000

#define T_P_GROUP 7000
#define R_M_L_GROUP 8000

#define STOP_AT_TEXT 9000


LARGE_INTEGER countsOnLastFrame;
LARGE_INTEGER currentCounts;
float frameTime = 0.0f;
float frequency = 100.0f;
int stopAtNum = 0;
char triggerText[4]="13";
char keyPressText[4] = "";
float countsToSeconds(__int64 a)
{
	LARGE_INTEGER temp;
	QueryPerformanceFrequency(&temp); // = counts/second
	return (float)(double(a)/double(temp.QuadPart));
}


LRESULT CALLBACK winCallBack(HWND hWnd, UINT msg, WPARAM wp, LPARAM lp);
void handleMessages()
{
	MSG msg;
	while(PeekMessage(&msg, NULL, 0, 0, PM_REMOVE))		
	{
		TranslateMessage(&msg);
		DispatchMessage	(&msg);
	}
}
int WINAPI WinMain(HINSTANCE instanceH, HINSTANCE prevInstanceH, LPSTR command_line, int show)
{
	ShowWindow(GetConsoleWindow(), SW_HIDE);
	//copy-pasta windows stuff below
	hInstance=instanceH;
	// Initializing the window class
	windClass.style			= CS_HREDRAW | CS_VREDRAW;
	windClass.lpfnWndProc		= winCallBack;
	windClass.cbClsExtra		= 0;
	windClass.cbWndExtra		= 0;
	windClass.hInstance		= instanceH;
	windClass.hIcon			= LoadIcon(NULL,IDI_APPLICATION);
	windClass.hCursor			= LoadCursor(NULL,IDC_ARROW);
	windClass.hbrBackground	= (HBRUSH)GetStockObject(WHITE_BRUSH);
	windClass.lpszClassName	= "Clicker";

	//Registering the window class
	RegisterClass(&windClass);

	hWnd=CreateWindow("Clicker","Clicker", WS_OVERLAPPEDWINDOW, 100, 150,150,460, NULL, NULL, instanceH, NULL);

	statusText = CreateWindow("Static","status: idle",WS_VISIBLE|WS_CHILD,5,1,135,35,hWnd,0,0,0);
	CreateWindow("Static","# clicks",WS_VISIBLE|WS_CHILD,5,40,70,20,hWnd,0,0,0);
	CreateWindow("Static","clicks/s",WS_VISIBLE|WS_CHILD,5,60,70,20,hWnd,0,0,0);
	CreateWindow("Static","trigger",WS_VISIBLE|WS_CHILD,5,80,70,20,hWnd,0,0,0);
	CreateWindow("Static","stop at",WS_VISIBLE|WS_CHILD,5,100,70,20,hWnd,0,0,0);

	char numStr[4];	
	_itoa_s(0x0D,numStr,4,10);

	char keyToPressStr[4];

	outputWindow =		CreateWindow("Edit","0",WS_VISIBLE|WS_CHILD|WS_BORDER|ES_READONLY,80,40,50,20,hWnd,(HMENU)OUTPUT_TEXT,0,0);
	inputFrequency =	CreateWindow("Edit","10",WS_VISIBLE|WS_CHILD|WS_BORDER,80,60,50,20,hWnd,(HMENU)INPUT_TEXT,0,0);
	stopAt =			CreateWindow("Edit","0",WS_VISIBLE|WS_CHILD|WS_BORDER,80,100,50,20,hWnd,(HMENU)STOP_AT_TEXT,0,0);
	triggerButton =		CreateWindow("Button",numStr,WS_VISIBLE|WS_CHILD|WS_BORDER,80,80,50,20,hWnd,(HMENU)TRIGGER_BTN,0,0);
	stopButton =		CreateWindow("Button","STOP!",WS_VISIBLE | WS_CHILD,5,125,100,50,hWnd,(HMENU)STOP_BTN,0,0);
	helpButton =		CreateWindow("Button","Help",WS_VISIBLE | WS_CHILD,5,180,100,50,hWnd,(HMENU)HELP_BTN,0,0);

	groupBox = CreateWindow("Button","press / toggle",WS_VISIBLE | WS_CHILD | BS_GROUPBOX,5,240,135,65,hWnd,(HMENU)T_P_GROUP,0,0);
	press = CreateWindow("Button","press",WS_VISIBLE | WS_CHILD | BS_AUTORADIOBUTTON | WS_GROUP,10,260,120,20,hWnd,(HMENU)T_P_GROUP,0,0);
	toggle = CreateWindow("Button","toggle",WS_VISIBLE | WS_CHILD | BS_AUTORADIOBUTTON,10,280,120,20,hWnd,(HMENU)T_P_GROUP,0,0);

	rmlGroupBox = CreateWindow("Button","mouse/key to click",WS_VISIBLE | WS_CHILD | BS_GROUPBOX,5,310,135,105,hWnd,(HMENU)R_M_L_GROUP,0,0);
	leftM = CreateWindow("Button","left",WS_VISIBLE | WS_CHILD | BS_AUTORADIOBUTTON | WS_GROUP,10,330,120,20,hWnd,(HMENU)R_M_L_GROUP,0,0);	
	middleM = CreateWindow("Button","middle",WS_VISIBLE | WS_CHILD | BS_AUTORADIOBUTTON,10,350,120,20,hWnd,(HMENU)R_M_L_GROUP,0,0);
	rightM = CreateWindow("Button","right",WS_VISIBLE | WS_CHILD | BS_AUTORADIOBUTTON,10,370,120,20,hWnd,(HMENU)R_M_L_GROUP,0,0);
	CreateWindow("Static", "key:", WS_VISIBLE | WS_CHILD, 10, 390, 40, 20, hWnd, 0, 0, 0);
	keyPressButton = CreateWindow("Button", "click to set", WS_VISIBLE | WS_CHILD | WS_BORDER, 45, 390, 80, 20, hWnd, (HMENU)KEY_PRESS_BTN, 0, 0);


	SendMessage(inputFrequency,EM_LIMITTEXT,4,0);
	SendMessage(doToggle?toggle:press,BM_CLICK,0,0);
	SendMessage(leftM,BM_CLICK,0,0);

	ShowWindow	(hWnd, show);
	UpdateWindow(hWnd);

	QueryPerformanceCounter(&countsOnLastFrame);

	memset(keyDown,0,VK_OEM_CLEAR);
	memset(keyPrev,0,VK_OEM_CLEAR);
	memset(keyTrig,0,VK_OEM_CLEAR);
	memset(keyUpTrig,0,VK_OEM_CLEAR);

	while(!quit)
	{			
		if (waitingForKey)
		{
			status = 3;
			if (status != prevStatus)
			{
				SetDlgItemText(hWnd, GetDlgCtrlID(statusText), "status: Hit the key that will be repeated.");
			}

			memset(keyDown, 0, VK_OEM_CLEAR);
			memset(keyTrig, 0, VK_OEM_CLEAR);
			memset(keyUpTrig, 0, VK_OEM_CLEAR);
			for (int i = 0;i<VK_OEM_CLEAR;i++)
			{
				if (GetAsyncKeyState(i))
				{
					keyDown[i] = 1;
					if (!keyPrev[i])
						keyTrig[i] = 1;
				}
				else if (keyPrev[i])
				{
					keyUpTrig[i] = 1;
				}

				keyPrev[i] = keyDown[i];
			}

			if (keyUpTrig[1] && !clickedOnceForKeyFlag)
			{
				clickedOnceForKeyFlag = true;
				continue;
			}

			//
			for (int i = 0;i<VK_OEM_CLEAR;i++)
			{
				if (keyUpTrig[i] && clickedOnceForKeyFlag)
				{
					_itoa_s(i, keyPressText, 4, 10);
					SetFocus(outputWindow);
					SetDlgItemText(hWnd, GetDlgCtrlID(keyPressButton), keyPressText);
					waitingForKey = false;
					clickedOnceForKeyFlag = false;
					SendMessage(leftM, BM_SETCHECK, BST_UNCHECKED, 0);
					SendMessage(middleM, BM_SETCHECK, BST_UNCHECKED, 0);
					SendMessage(rightM, BM_SETCHECK, BST_UNCHECKED, 0);
					break;
				}
			}
			//
		}
		else if(waitingForTrigger)
		{
			status = 1;
			if(status != prevStatus)
			{
				SetDlgItemText(hWnd,GetDlgCtrlID(statusText),"status: please hit trigger key");				
			}

			memset(keyDown,0,VK_OEM_CLEAR);
			memset(keyTrig,0,VK_OEM_CLEAR);
			memset(keyUpTrig,0,VK_OEM_CLEAR);
			for(int i=0;i<VK_OEM_CLEAR;i++)
			{
				if(GetAsyncKeyState(i))
				{
					keyDown[i]=1;
					if(!keyPrev[i])
						keyTrig[i]=1;
				}
				else if(keyPrev[i])
				{
					keyUpTrig[i]=1;				
				}

				keyPrev[i] = keyDown[i];				 
			}	

			if(keyUpTrig[1] && !clickedOnceForTriggerFlag)
			{
				clickedOnceForTriggerFlag = true;
				continue;
			}

			for(int i=0;i<VK_OEM_CLEAR;i++)
			{
				sameTriggerAndClick=false;
				if(keyUpTrig[i] && clickedOnceForTriggerFlag)
				{
					switch(mouseToClick)
					{
					case 0:
						if(i==VK_LBUTTON)
							sameTriggerAndClick=true;
						break;
					case 1:
						if(i==VK_MBUTTON)
							sameTriggerAndClick=true;
						break;
					case 2:
						if(i==VK_RBUTTON)
							sameTriggerAndClick=true;
						break;
					};
					_itoa_s(i,triggerText,4,10);
					SetFocus(outputWindow);
					SetDlgItemText(hWnd,GetDlgCtrlID(triggerButton),triggerText);
					waitingForTrigger = false;
					clickedOnceForTriggerFlag = false;
					break;
				}
			}
			if(waitingForTrigger == false)
				continue;
		}
		else
		{

			QueryPerformanceCounter(&currentCounts);
			frameTime=countsToSeconds(currentCounts.QuadPart-countsOnLastFrame.QuadPart);

			char numStr[12];

			//to ensure no problems with starting and stopping the toggle state:
			if(toggleState == 0 && GetAsyncKeyState(atoi(triggerText)))
				toggleState = 1;
			if(toggleState == 1 && !GetAsyncKeyState(atoi(triggerText)))
				toggleState = 2;
			if(toggleState == 2 && GetAsyncKeyState(atoi(triggerText)))
				toggleState = 3;
			if(toggleState == 3 && !GetAsyncKeyState(atoi(triggerText)))
				toggleState = 0;

			if(frameTime>1.0f/(frequency*2.0f))
			{
				if(stopAtNum > 0 && numClicksSinceStop >= stopAtNum)
				{
					if(doToggle)
					{
						numClicksSinceStop = 0;
						toggleState = 0;
					}
					else
					{
						waitingForTriggerUp = true;
					}
				}

				if((doToggle && toggleState>=2) || !doToggle && GetAsyncKeyState(atoi(triggerText)) && !waitingForTriggerUp)
				{
					status = 2;
					if(status != prevStatus)
					{
						SetDlgItemText(hWnd,GetDlgCtrlID(statusText),"status: clicking");
					}

					GetDlgItemText(hWnd, GetDlgCtrlID(keyPressButton), numStr, 11);
					int keyToPress = atoi(numStr);

					if(switchFlag && keyToPress <= 0)
					{
						switch(mouseToClick)
						{
						case 0:
							mouse_event(MOUSEEVENTF_LEFTDOWN,0,0,0,0);
							break;
						case 1:
							mouse_event(MOUSEEVENTF_MIDDLEDOWN,0,0,0,0);
							break;
						case 2:
							mouse_event(MOUSEEVENTF_RIGHTDOWN,0,0,0,0);
							break;
						};
						
						switchFlag=false;
					}
					else
					{
						if (keyToPress > 0)
						{
							keybd_event(keyToPress, keyToPress, 0, 0);
						}
						else
						{
							switch (mouseToClick)
							{
							case 0:
								mouse_event(MOUSEEVENTF_LEFTUP, 0, 0, 0, 0);
								break;
							case 1:
								mouse_event(MOUSEEVENTF_MIDDLEUP, 0, 0, 0, 0);
								break;
							case 2:
								mouse_event(MOUSEEVENTF_RIGHTUP, 0, 0, 0, 0);
								break;
							};
						}
						
						switchFlag=true;
						numClicks++;
						numClicksSinceStop++;
						_itoa_s(numClicks,numStr,12,10);
						SetDlgItemText(hWnd,GetDlgCtrlID(outputWindow),numStr);

					}			
				}
				else if(!switchFlag)
				{
					switch(mouseToClick)
					{
					case 0:
						mouse_event(MOUSEEVENTF_LEFTUP,0,0,0,0);
						break;
					case 1:
						mouse_event(MOUSEEVENTF_MIDDLEUP,0,0,0,0);
						break;
					case 2:
						mouse_event(MOUSEEVENTF_RIGHTUP,0,0,0,0);
						break;
					};
					switchFlag=true;
					numClicks++;
					numClicksSinceStop++;
					_itoa_s(numClicks,numStr,12,10);
					SetDlgItemText(hWnd,GetDlgCtrlID(outputWindow),numStr);
				}
				else
				{
					status = 0;
					numClicksSinceStop = 0;
					if(!GetAsyncKeyState(atoi(triggerText)))
					{
						waitingForTriggerUp = false;
					}
					if(status != prevStatus)
					{
						SetDlgItemText(hWnd,GetDlgCtrlID(statusText),"status: idle");
					}
				}
				countsOnLastFrame=currentCounts;
			}
			else
			{
				Sleep(1);
			}

			GetDlgItemText(hWnd,GetDlgCtrlID(inputFrequency),numStr,11);
			frequency = float(atof(numStr));
			
			GetDlgItemText(hWnd,GetDlgCtrlID(stopAt),numStr,11);
			stopAtNum = float(atof(numStr));

			//if(frequency < 1.0f)
			//{
			//	frequency = 1.0f;
			//	SetDlgItemText(hWnd,GetDlgCtrlID(inputFrequency),"1");
			//}
		}
		handleMessages();
		prevStatus = status;
	}
	return 0;
}

LRESULT CALLBACK winCallBack(HWND hWin, UINT msg, WPARAM wp, LPARAM lp) 
{
	HDC dc;   
	PAINTSTRUCT ps;
	switch (msg)
	{
	case WM_COMMAND:
		switch(LOWORD(wp))
		{
		case HELP_BTN:
			MessageBox(hWnd,"-Topmost text field indicates number of clicks\n"
				            "-Middle text field is the frequency of the clicks in clicks per second.\n"
							" Frequency can be as high as 9999 c/s\n"
							" You may enter fractional frequencies. For example, 0.5 is one click every two seconds.\n"
							"-Below that, the button is the trigger key. Click on it and then type a key (or hit a mouse button).\n"
							" That key will then trigger the mouse clicks when pressed.\n"
							" Note: You can't have the same mouse button be the trigger and clicker.\n"
							" Note: You can't change the trigger if you chose the left mouse button; you must restart the program.\n"
							" Default (13) is the enter key.\n"
							"-The lowest text field is the number of clicks before the clicking automatically stops.\n"
							" 0 is default and means infinity.\n"
							"-The STOP! button stops toggled clicking.\n"
							"-The trigger key still works when this program is minimized.\n"
							"-You must close this program to stop a trigger key from clicking.\n\n"
							"Use at your own risk, I assume no liability for any damage this program causes.\n\n"
							"Programming by Boris Mezhibovskiy\nborman500@msn.com","Help",0);
			break;
		case R_M_L_GROUP:
			SetDlgItemText(hWnd, GetDlgCtrlID(keyPressButton), "");
			mouseToClick=0;
			if((HWND)lp == leftM)
			{
				mouseToClick = 0;
			}
			else if((HWND)lp == middleM)
			{
				mouseToClick = 1;
			}
			else if((HWND)lp == rightM)
			{
				mouseToClick = 2;
			}
			break;
		case T_P_GROUP:
			toggleState=0;
			if((HWND)lp == toggle)
			{
				doToggle = true;
			}
			else if((HWND)lp == press)
			{
				doToggle = false;
			}			
			break;
		case STOP_BTN:
			toggleState = 0;
			break;
		case TRIGGER_BTN:
			if(status!=2 && !waitingForTrigger)
			{
				toggleState = 0;
				waitingForTrigger = true;
			}
			break;
		case KEY_PRESS_BTN:
			if (status != 2 && !waitingForKey)
			{
				toggleState = 0;
				waitingForKey = true;
			}
			break;
		}
		break;

	case WM_CREATE:	
		InvalidateRect(hWnd,NULL,TRUE);
		break;
	case WM_SIZE:
		InvalidateRect(hWnd,NULL,TRUE);
		break;
	case WM_PAINT:

		dc = BeginPaint(hWin, &ps);
		EndPaint(hWin, &ps);
		break;
		// When it's time for the window to go away
	case WM_DESTROY:
		quit=true;
		PostQuitMessage(0);
		break;
		// called any time the window is moved
	case WM_MOVE:
		// Invalidate the rect to force a redraw
		InvalidateRect(hWin, NULL, TRUE);
		break;
	default:
		return DefWindowProc(hWin, msg, wp, lp);

	}
	return 0;
}