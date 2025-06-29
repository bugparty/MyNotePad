#pragma once
#include "pch.h"
using CWindowTraits = CWinTraits<WS_OVERLAPPEDWINDOW | WS_CLIPCHILDREN | WS_CLIPSIBLINGS, WS_EX_CLIENTEDGE>;


// Global variables for Find functionality
static HWND g_hFindDialog = NULL;
static HWND g_hReplaceDialog = NULL;
static HWND g_hGoToDialog = NULL;
static HWND g_hEditControl = NULL;
static TCHAR g_szFindText[256] = { 0 };
static TCHAR g_szReplaceText[256] = { 0 };
static BOOL g_bMatchCase = FALSE;
static BOOL g_bWholeWord = FALSE;

// Font related global variables
static LOGFONT g_currentLogFont = { 0 };
static BOOL g_bFontInitialized = FALSE;

// Word Wrap related global variables
static BOOL g_bWordWrapEnabled = FALSE;

// Original edit control window procedure
static WNDPROC g_pOriginalEditProc = NULL;

// Global resources for edit control styling
static HFONT g_hEditFont = NULL;
static HBRUSH g_hEditBrush = NULL;

class CMainWindow : public CFrameWindowImpl <CMainWindow,CWindow, CWindowTraits>
{
public:
	DECLARE_FRAME_WND_CLASS(_T("NotePadWTL"), IDI_NOTEPADWTL);
	CEdit m_editCtrl;
	CMainWindow() {}
	BEGIN_MSG_MAP(CMainWindow)
		MESSAGE_HANDLER(WM_SIZE, OnPaint)
		MESSAGE_HANDLER(WM_CLOSE, OnClose)
		MESSAGE_HANDLER(WM_DESTROY, OnDestroy)
		MESSAGE_HANDLER(WM_CREATE, OnCreate)
		COMMAND_ID_HANDLER(IDM_EDIT_COPY, OnCopy)
		COMMAND_ID_HANDLER(IDM_EDIT_CUT, OnCut)
		COMMAND_ID_HANDLER(IDM_EDIT_PASTE, OnPaste)
		COMMAND_ID_HANDLER(IDM_EDIT_DEL, OnDelete)


	END_MSG_MAP()

	// Creation of main EditControl with enhanced appearance
	VOID CreateEditControl() {
		// Set edit control style based on Word Wrap status
		DWORD dwStyle = WS_CHILD | WS_VISIBLE | WS_VSCROLL | ES_MULTILINE |
			ES_LEFT | ES_WANTRETURN | ES_AUTOVSCROLL;

		if (!g_bWordWrapEnabled) {
			// If Word Wrap is not enabled, add horizontal scroll bar
			dwStyle |= WS_HSCROLL | ES_AUTOHSCROLL;
		}
		CRect rcClient;
		GetClientRect(&rcClient);
		// 使用 URect 进行偏移或缩放
		rcClient.DeflateRect(10, 10);
		m_editCtrl.Create(
			m_hWnd,
			rcDefault, // Initial size, will be adjusted in WM_SIZE
			_T(""),
			dwStyle,
			0, (HMENU)ID_EDIT,
			0);
			 
             /* pointer not needed                */

		// Subclass the edit control for custom keyboard handling and appearance
		// if (m_editCtrl) {
		// 	g_pOriginalEditProc = (WNDPROC)SetWindowLongPtr(m_editCtrl, GWLP_WNDPROC, (LONG_PTR)EditControlSubclassProc);
		//
		// 	// Initialize default font settings
		// 	//InitializeDefaultFont();
		//
		// 	// Create font from LOGFONT if not already created
		// 	if (!g_hEditFont) {
		// 		g_hEditFont = CreateFontIndirect(&g_currentLogFont);
		// 	}
		//
		// 	// Create background brush if not already created
		// 	if (!g_hEditBrush) {
		// 		g_hEditBrush = CreateSolidBrush(RGB(255, 255, 255)); // Pure white background
		// 	}
		//
		// 	// Set the font
		// 	if (g_hEditFont) {
		// 		SendMessage(m_editCtrl, WM_SETFONT, (WPARAM)g_hEditFont, TRUE);
		// 	}
		//
		// 	// Add generous margins for better readability
		// 	SendMessage(m_editCtrl, EM_SETMARGINS, EC_LEFTMARGIN | EC_RIGHTMARGIN, MAKELPARAM(12, 12));
		//
		// 	// Set tab stops for better formatting (every 4 characters)
		// 	int tabStops = 32; // 4 characters * 8 units per character
		// 	SendMessage(m_editCtrl, EM_SETTABSTOPS, 1, (LPARAM)&tabStops);
		//
		// 	// Apply additional modern styling
		// 	//ApplyModernEditStyling(hwndEdit);
		// }
	}
	LRESULT OnCopy(WORD wNotifyCode, WPARAM wParam, HWND /*lParam*/, BOOL& /*bHandled*/)
	{
		m_editCtrl.Copy();
		return 0;
	}
	LRESULT OnCut(WORD wNotifyCode, WPARAM wParam, HWND /*lParam*/, BOOL& /*bHandled*/)
	{
		m_editCtrl.Cut();
		return 0;
	}
	LRESULT OnPaste(WORD wNotifyCode, WPARAM wParam, HWND /*lParam*/, BOOL& /*bHandled*/)
	{
		m_editCtrl.Paste();
		return 0;
	}
	LRESULT OnDelete(WORD wNotifyCode, WPARAM wParam, HWND /*lParam*/, BOOL& /*bHandled*/)
	{
		m_editCtrl.Clear();
		return 0;
	}
	LRESULT OnCreate(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/)
	{
		HMENU hMenu = LoadMenu(_Module.GetResourceInstance(), MAKEINTRESOURCE(IDC_MYNOTEPAD));
		SetMenu(hMenu);

		CreateEditControl();
		return 0;
	}
	LRESULT OnPaint(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/)
	{
		PAINTSTRUCT ps;
		HDC hdc = BeginPaint(&ps);
		if (m_editCtrl)
		{
			RECT rc;
			GetClientRect(&rc);
			FillRect(hdc, &rc, (HBRUSH)(COLOR_WINDOW + 1));
			SendMessage(m_editCtrl, WM_PAINT, (WPARAM)hdc, 0);
		}
		EndPaint(&ps);
		return 0;
	}
	LRESULT OnDestroy(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/)
	{
		PostQuitMessage(0);
		return 0;
	}
	LRESULT OnClose(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/)
	{
		if (MessageBox(_T("Are you sure you want to exit?"), _T("Exit"), MB_YESNO | MB_ICONQUESTION) == IDYES)
		{
			DestroyWindow();
		}
		return 0;
	}
private:
	
};