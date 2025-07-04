﻿// MyNotePad.cpp : Defines the entry point for the application.
//

#include "stdafx.h"
#include "MyNotePad.h"
#include "Dialog.h"
#include "FileDialog.h"
#include "Print.h"
#include "StatusBar.h"
#include "AboutDialog.h"

#define MAX_LOADSTRING 100

// Global Variables: 
HINSTANCE hInst;								// Current instance
HWND hWnd;										// Current window handle
HWND hwndEdit;									// Edit control handle
TCHAR szAppTitle[MAX_LOADSTRING];				// The title bar text
TCHAR szWindowClass[MAX_LOADSTRING];			// the main window class name
TCHAR szFailedToLoadCommCtl[MAX_LOADSTRING];

// Undo/Redo state tracking
static BOOL g_bLastWasUndo = FALSE;				// Track if last operation was undo


// Forward declarations of functions included in this code module: 
ATOM				MyRegisterClass(HINSTANCE hInstance);
BOOL				InitInstance(HINSTANCE, int);
LRESULT CALLBACK	WndProc(HWND, UINT, WPARAM, LPARAM);
INT_PTR CALLBACK	About(HWND, UINT, WPARAM, LPARAM);
VOID				InitStrRes();
VOID				ResetUndoState();
VOID				ProcessCommandLine(LPTSTR lpCmdLine);

#define nEditID 40001

int APIENTRY _tWinMain(_In_ HINSTANCE hInstance,
                     _In_opt_ HINSTANCE hPrevInstance,
                     _In_ LPTSTR    lpCmdLine,
                     _In_ int       nCmdShow)
{
	UNREFERENCED_PARAMETER(hPrevInstance);

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

	// Process command line arguments to open file if specified
	ProcessCommandLine(lpCmdLine);

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
	int editHeight;
	switch (message)
	{
	case WM_COMMAND:
		if (lParam){
			if (LOWORD(lParam) == ID_EDIT && (HIWORD(wParam) == EN_ERRSPACE || HIWORD(wParam) == EN_MAXTEXT)){
				MessageBox(hWnd, _T("Edit control out of space."), szAppTitle, MB_OK | MB_ICONSTOP);
				return 0;
			}
			// Reset undo state when user types or makes changes
			if (LOWORD(lParam) == ID_EDIT && HIWORD(wParam) == EN_CHANGE) {
				ResetUndoState();
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
			ShowPrintDialog(hWnd, hwndEdit);
			return 0;
		case IDM_FILE_PRINT_SETTING:
			ShowPageSetupDialog(hWnd);
			return 0;
		case IDM_EDIT_UNDO:
			// Only perform undo if we can undo and it's not a redo state
			if (SendMessage(hwndEdit, EM_CANUNDO, 0, 0)) {
				SendMessage(hwndEdit, WM_UNDO, 0, 0);
				g_bLastWasUndo = TRUE;
			}
			return 0;
		case IDM_EDIT_REDO:
			// Only perform redo if last operation was undo and we can still undo
			if (g_bLastWasUndo && SendMessage(hwndEdit, EM_CANUNDO, 0, 0)) {
				SendMessage(hwndEdit, WM_UNDO, 0, 0);
				g_bLastWasUndo = FALSE;
			}
			return 0;
		case IDM_EDIT_COPY:
			SendMessage(hwndEdit, WM_COPY, 0, 0);
			return 0;
		case IDM_EDIT_CUT:
			SendMessage(hwndEdit, WM_CUT, 0, 0);
			ResetUndoState();
			return 0;
		case IDM_EDIT_PASTE:
			SendMessage(hwndEdit, WM_PASTE, 0, 0);
			ResetUndoState();
			return 0;
		case IDM_EDIT_DEL:
			SendMessage(hwndEdit, WM_CLEAR, 0, 0);
			ResetUndoState();
			return 0;
		case IDM_EDIT_SELECTALL:
			SelectAllText(hwndEdit);
			return 0;
		case IDM_EDIT_INSERTDATE:
			InsertDateTime(hwndEdit);
			return 0;
		case IDM_EDIT_FIND:
			ShowFindDialog(hWnd,hwndEdit);
			return 0;
		case IDM_EDIT_FINDNEXT:
			FindNext(hwndEdit);
			return 0;
		case IDM_EDIT_REPLACE:
			ShowReplaceDialog(hWnd, hwndEdit);
			return 0;
		case IDM_EDIT_GOTO:
			ShowGoToDialog(hWnd, hwndEdit);
			return 0;
		case IDM_FORMAT_FONT:
			ShowFontDialog(hWnd, hwndEdit);
			return 0;
		case IDM_FORMAT_AUTOWRAP:
			ToggleWordWrap(hWnd, hwndEdit);
			return 0;
		case IDM_VIEW_STATUSBAR:
			ToggleStatusBar(hWnd);
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
		DestroyStatusBar(); // Clean up status bar resources
		PostQuitMessage(0);
		return 0;
		break;


	case WM_CREATE:
		hdc = GetDC(hWnd);
		GetTextMetrics(hdc, &tm);
		ReleaseDC(hWnd, hdc);

		// 初始化 Word Wrap 设置
		InitializeWordWrap();
		
		// 初始化状态栏设置
		InitializeStatusBar();
		
		CreateEditControl(hwndEdit, hWnd);
		
		// 创建状态栏
		CreateStatusBar(hWnd);
		
		// 初始化菜单状态
		UpdateMenuWordWrap(hWnd);
		UpdateMenuStatusBar(hWnd);
		
		// 确保状态栏显示正确的 Word Wrap 状态
		UpdateStatusBarWordWrap(IsWordWrapEnabled());
		
		return 0;

		break;

	case WM_SIZE:
		cxClient = LOWORD(lParam);
		cyClient = HIWORD(lParam);

		// 更新状态栏布局
		UpdateStatusBarLayout(hWnd);
		
		// 计算编辑控件的区域（需要排除状态栏的高度）
		editHeight = cyClient;
		if (IsStatusBarVisible()) {
			HWND hStatusBar = GetStatusBarHandle();
			if (hStatusBar && IsWindow(hStatusBar)) {
				RECT rcStatus;
				GetClientRect(hStatusBar, &rcStatus); // 使用 GetClientRect 而不是 GetWindowRect
				editHeight -= (rcStatus.bottom - rcStatus.top);
			}
		}

		MoveWindow(hwndEdit, 0, 0, cxClient, editHeight, TRUE);
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

	case WM_INITMENUPOPUP:
		{
			HMENU hMenu = (HMENU)wParam;
			HMENU hEditMenu = GetSubMenu(GetMenu(hWnd), 1); // Edit menu is typically the second menu (index 1)
			
			if (hMenu == hEditMenu) {
				// Check undo/redo availability
				BOOL canUndo = SendMessage(hwndEdit, EM_CANUNDO, 0, 0);
				
				// Enable Undo if we can undo and it's not in redo state
				BOOL enableUndo = canUndo && !g_bLastWasUndo;
				EnableMenuItem(hMenu, IDM_EDIT_UNDO, MF_BYCOMMAND | (enableUndo ? MF_ENABLED : MF_GRAYED));
				
				// Enable Redo if last operation was undo and we can still undo (meaning there's something to redo)
				BOOL enableRedo = g_bLastWasUndo && canUndo;
				EnableMenuItem(hMenu, IDM_EDIT_REDO, MF_BYCOMMAND | (enableRedo ? MF_ENABLED : MF_GRAYED));
				
				// Check if there's a selection for cut/copy/delete
				DWORD selStart, selEnd;
				SendMessage(hwndEdit, EM_GETSEL, (WPARAM)&selStart, (LPARAM)&selEnd);
				BOOL hasSelection = (selStart != selEnd);
				EnableMenuItem(hMenu, IDM_EDIT_CUT, MF_BYCOMMAND | (hasSelection ? MF_ENABLED : MF_GRAYED));
				EnableMenuItem(hMenu, IDM_EDIT_COPY, MF_BYCOMMAND | (hasSelection ? MF_ENABLED : MF_GRAYED));
				EnableMenuItem(hMenu, IDM_EDIT_DEL, MF_BYCOMMAND | (hasSelection ? MF_ENABLED : MF_GRAYED));
				
				// Check if clipboard has text for paste
				BOOL canPaste = IsClipboardFormatAvailable(CF_TEXT) || IsClipboardFormatAvailable(CF_UNICODETEXT);
				EnableMenuItem(hMenu, IDM_EDIT_PASTE, MF_BYCOMMAND | (canPaste ? MF_ENABLED : MF_GRAYED));
			}
		}
		return 0;

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

// Reset undo state when new edit operations occur
VOID ResetUndoState() {
	g_bLastWasUndo = FALSE;
}

// Process command line arguments
VOID ProcessCommandLine(LPTSTR lpCmdLine) {
	if (!lpCmdLine || lpCmdLine[0] == '\0') {
		return; // No command line arguments
	}
	
	// Remove leading and trailing whitespace
	LPTSTR pStart = lpCmdLine;
	while (*pStart == ' ' || *pStart == '\t') {
		pStart++;
	}
	
	if (*pStart == '\0') {
		return; // Only whitespace
	}
	
	// Copy to a local buffer for processing
	TCHAR szFileName[MAX_PATH];
	_tcscpy_s(szFileName, MAX_PATH, pStart);
	
	// Remove trailing whitespace
	int len = _tcslen(szFileName);
	while (len > 0 && (szFileName[len - 1] == ' ' || szFileName[len - 1] == '\t')) {
		szFileName[--len] = '\0';
	}
	
	// Handle quoted filenames
	if (szFileName[0] == '"') {
		// Remove opening quote
		_tcscpy_s(szFileName, MAX_PATH, szFileName + 1);
		len = _tcslen(szFileName);
		
		// Remove closing quote if present
		if (len > 0 && szFileName[len - 1] == '"') {
			szFileName[len - 1] = '\0';
		}
	}
	
	// Try to open the file
	if (szFileName[0] != '\0') {
		OpenFileFromCommandLine(hwndEdit, szFileName);
	}
}

