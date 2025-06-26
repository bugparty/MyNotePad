// MyNotePad.cpp : Defines the entry point for the application.
//

#include "stdafx.h"
#include "MyNotePad.h"
#include "Dialog.h"

#define MAX_LOADSTRING 100

// Global Variables: 
HINSTANCE hInst;								// Current instance
HWND hWnd;										// Current window handle
TCHAR szAppTitle[MAX_LOADSTRING];					// The title bar text
TCHAR szWindowClass[MAX_LOADSTRING];			// the main window class name
TCHAR szFailedToLoadCommCtl[MAX_LOADSTRING];


// Forward declarations of functions included in this code module: 
ATOM				MyRegisterClass(HINSTANCE hInstance);
BOOL				InitInstance(HINSTANCE, int);
LRESULT CALLBACK	WndProc(HWND, UINT, WPARAM, LPARAM);
INT_PTR CALLBACK	About(HWND, UINT, WPARAM, LPARAM);
VOID InitStrRes();

#define nEditID 40001

int APIENTRY _tWinMain(_In_ HINSTANCE hInstance,
                     _In_opt_ HINSTANCE hPrevInstance,
                     _In_ LPTSTR    lpCmdLine,
                     _In_ int       nCmdShow)
{
	UNREFERENCED_PARAMETER(hPrevInstance);
	UNREFERENCED_PARAMETER(lpCmdLine);

 	// TODO: Place code here.
	MSG msg;
	HACCEL hAccelTable;

	// Initialize global strings
	InitStrRes();
	MyRegisterClass(hInstance);

	// Perform application initialization: 
	if (!InitInstance (hInstance, nCmdShow))
	{
		return FALSE;
	}

	hAccelTable = LoadAccelerators(hInstance, MAKEINTRESOURCE(IDC_MYNOTEPAD));

	// Main message loop: 
	while (GetMessage(&msg, NULL, 0, 0))
	{
		if (!TranslateAccelerator(msg.hwnd, hAccelTable, &msg))
		{
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}
	}

	return (int) msg.wParam;
}



//
//  FUNCTION: MyRegisterClass()
//
//  PURPOSE: Registers the window class.
//
ATOM MyRegisterClass(HINSTANCE hInstance)
{
	WNDCLASSEX wcex;

	wcex.cbSize = sizeof(WNDCLASSEX);

	wcex.style			= CS_HREDRAW | CS_VREDRAW;
	wcex.lpfnWndProc	= WndProc;
	wcex.cbClsExtra		= 0;
	wcex.cbWndExtra		= 0;
	wcex.hInstance		= hInstance;
	wcex.hIcon			= LoadIcon(hInstance, MAKEINTRESOURCE(IDI_MYNOTEPAD));
	wcex.hCursor		= LoadCursor(NULL, IDC_ARROW);
	wcex.hbrBackground	= (HBRUSH)(COLOR_WINDOW+1);
	wcex.lpszMenuName	= MAKEINTRESOURCE(IDC_MYNOTEPAD);
	wcex.lpszClassName	= szWindowClass;
	wcex.hIconSm		= LoadIcon(wcex.hInstance, MAKEINTRESOURCE(IDI_SMALL));

	return RegisterClassEx(&wcex);
}

//
//   FUNCTION: InitInstance(HINSTANCE, int)
//
//   PURPOSE: Saves instance handle and creates main window
//
//   COMMENTS:
//
//        In this function, we save the instance handle in a global variable and
//        create and display the main program window.
//
BOOL InitInstance(HINSTANCE hInstance, int nCmdShow)
{

   hInst = hInstance; // Store instance handle in our global variable

   hWnd = CreateWindow(szWindowClass, szAppTitle, WS_OVERLAPPEDWINDOW,
      CW_USEDEFAULT, 0, CW_USEDEFAULT, 0, NULL, NULL, hInstance, NULL);

   if (!hWnd)
   {
      return FALSE;
   }

   INITCOMMONCONTROLSEX iccx;
   iccx.dwSize = sizeof(INITCOMMONCONTROLSEX);
   iccx.dwICC = ICC_BAR_CLASSES;
   if (!InitCommonControlsEx(&iccx)){
	   MessageBox(hWnd, szFailedToLoadCommCtl, szAppTitle, MB_OK);
	   return FALSE;
   }

   ShowWindow(hWnd, nCmdShow);
   UpdateWindow(hWnd);

   return TRUE;
}

//
//  FUNCTION: WndProc(HWND, UINT, WPARAM, LPARAM)
//
//  PURPOSE:    Processes messages for the main window.
//
//  WM_COMMAND	- process the application menu
//  WM_PAINT	- Paint the main window
//  WM_DESTROY	- post a quit message and return
//
//
LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	int wmId, wmEvent;
	PAINTSTRUCT ps;
	HDC hdc;
	TEXTMETRIC tm;
	static int cxChar, cyChar, cxClient, cyClient;
	static int iVertPos;
	static SCROLLINFO si;
	static HWND hwndEdit;
	switch (message)
	{
	case WM_COMMAND:
		if (lParam){
			if (LOWORD(lParam) == ID_EDIT && (HIWORD(wParam) == EN_ERRSPACE || HIWORD(wParam) == EN_MAXTEXT)){
				MessageBox(hWnd, _T("Edit control out of space."), szAppTitle, MB_OK | MB_ICONSTOP);
				return 0;
			}

		}
		wmId = LOWORD(wParam);
		wmEvent = HIWORD(wParam);
		// Parse the menu selections: 
		switch (wmId)
		{
		case IDM_FILE_OPEN:
			OpenDialogFileOpen(hWnd,hwndEdit);
			return 0;
		case IDM_FILE_SAVE:
			OpenDialogFileSave(hWnd, hwndEdit);
			return 0;
		case IDM_FILE_NEW:

		case IDM_FILE_SAVEAS:
			OpenDialogFileSaveAs(hWnd, hwndEdit);
			return 0;
		case IDM_FILE_PRINT:
			MessageBeep(0);
			return 0;
		case IDM_EDIT_UNDO:
			SendMessage(hwndEdit, WM_UNDO, 0, 0);
			return 0;
		case IDM_EDIT_COPY:
			SendMessage(hwndEdit, WM_COPY, 0, 0);
			return 0;
		case IDM_EDIT_CUT:
			SendMessage(hwndEdit, WM_CUT, 0, 0);
			return 0;
		case IDM_EDIT_PASTE:
			SendMessage(hwndEdit, WM_PASTE, 0, 0);
			return 0;
		case IDM_EDIT_DEL:
			SendMessage(hwndEdit, WM_CLEAR, 0, 0);
			return 0;
		case IDM_EDIT_FIND:
			ShowFindDialog(hWnd, hwndEdit);
			return 0;
		case IDM_EDIT_REPLACE:
			ShowReplaceDialog(hWnd, hwndEdit);
			return 0;
		case IDM_HELP_ABOUT:
			DialogBox(hInst, MAKEINTRESOURCE(IDD_ABOUTBOX), hWnd, About);
			break;
		case IDM_EXIT:
			DestroyWindow(hWnd);
			break;
		default:
			return DefWindowProc(hWnd, message, wParam, lParam);
		}
		return 0;
		break;
	case WM_DESTROY:
		CleanupEditResources(); // Clean up edit control resources
		PostQuitMessage(0);
		return 0;
		break;


	case WM_CREATE:
		hdc = GetDC(hWnd);
		GetTextMetrics(hdc, &tm);
		ReleaseDC(hWnd, hdc);

		CreateEditControl(hwndEdit, hWnd);
		return 0;

		break;

	case WM_SIZE:
		cxClient = LOWORD(lParam);
		cyClient = HIWORD(lParam);

		MoveWindow(hwndEdit, 0, 0, cxClient, cyClient, TRUE);
		// Set vertical scroll bar range and page size
		si.cbSize = sizeof(si);
		si.fMask = SIF_RANGE | SIF_PAGE;
		si.nMin = 0;
		si.nMax = 50;
		if (cyChar == 0) {
			return 0;
		}
		si.nPage = cyClient / cyChar;

		//SetScrollInfo should be the last line because it will endup the callback immedately
		SetScrollInfo(hWnd, SB_VERT, &si, TRUE);

		return 0;
		break;

	case WM_PAINT:
		hdc = BeginPaint(hWnd, &ps);
		// TODO: Add any drawing code here...


		EndPaint(hWnd, &ps);
		return 0;
		break;

	case WM_CTLCOLOREDIT:
		{
			// Handle color messages for the edit control
			HDC hdc = (HDC)wParam;
			HWND hEdit = (HWND)lParam;
			
			// Set custom colors for better appearance
			SetTextColor(hdc, RGB(33, 37, 41));        // Dark gray text (modern)
			SetBkColor(hdc, RGB(255, 255, 255));       // Pure white background
			
			// Create and return a brush for the background
			static HBRUSH hEditBrush = CreateSolidBrush(RGB(255, 255, 255));
			return (LRESULT)hEditBrush;
		}

	case WM_VSCROLL:
		si.cbSize = sizeof(si);
		si.fMask = SIF_ALL;
		//Save the position for comparision later
		GetScrollInfo(hWnd, SB_VERT, &si);
		iVertPos = si.nPos;

		switch (LOWORD(wParam)){
		case SB_TOP:
			si.nPos = si.nMin;
			break;
		case SB_BOTTOM:
			si.nPos = si.nMax;
			break;
		case SB_LINEUP:
			si.nPos -= 1;
			break;
		case SB_LINEDOWN:
			si.nPos += 1;
			break;
		case SB_PAGEUP:
			si.nPos -= si.nPage;
			break;
		case SB_PAGEDOWN:
			si.nPos += si.nPage;
			break;
		case SB_THUMBTRACK:
			si.nPos = si.nTrackPos;
			break;
		default:
			break;

		}
		// Set the position and then retrieve it.  Due to adjustments
		//  by Windows it may not be the same as the value set
		si.fMask = SIF_POS;
		SetScrollInfo(hWnd, SB_VERT, &si, TRUE);
		GetScrollInfo(hWnd, SB_VERT, &si);
		// If the position has changed, scroll the window and update it
		if (si.nPos != iVertPos)
		{
			ScrollWindow(hWnd, 0, cyChar * (iVertPos - si.nPos),
				NULL, NULL);
			UpdateWindow(hWnd);
		}
		return 0;
		break;
	default:
		return DefWindowProc(hWnd, message, wParam, lParam);

	}
}
//function to  Initialize global strings
VOID InitStrRes(){
	// Initialize global strings
	LoadString(hInst, IDC_MYNOTEPAD, szWindowClass, MAX_LOADSTRING);
	LoadString(hInst, IDS_APP_TITLE, szAppTitle, MAX_LOADSTRING);
	LoadString(hInst, IDS_FAILED_TO_INIT_COMM_CTL, szFailedToLoadCommCtl, MAX_LOADSTRING);
}

