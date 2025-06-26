#include "stdafx.h"
#include "Dialog.h"
#include "resource.h"
#include "Utils.h"
#include "Document.h"

// External global variables
extern HINSTANCE hInst;

// Global variable to store current file name
static TCHAR g_szCurrentFileName[MAX_PATH] = {0};
static BOOL g_bHasFileName = FALSE;

// Global variables for Find functionality
static HWND g_hFindDialog = NULL;
static HWND g_hReplaceDialog = NULL;
static HWND g_hEditControl = NULL;
static TCHAR g_szFindText[256] = {0};
static TCHAR g_szReplaceText[256] = {0};
static BOOL g_bMatchCase = FALSE;
static BOOL g_bWholeWord = FALSE;

// Original edit control window procedure
static WNDPROC g_pOriginalEditProc = NULL;

// Global resources for edit control styling
static HFONT g_hEditFont = NULL;
static HBRUSH g_hEditBrush = NULL;

// 打印相关全局变量
static PRINTDLG g_printDlg = {0};
static PAGESETUPDLG g_pageSetupDlg = {0};
static BOOL g_bPrintInitialized = FALSE;

// Subclassed edit control window procedure
LRESULT CALLBACK EditControlSubclassProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam) {
	switch (message) {
	case WM_GETDLGCODE:
		{
			// Tell the system we want to handle all keys, including Enter and Tab
			return DLGC_WANTALLKEYS | DLGC_HASSETSEL | DLGC_WANTCHARS | DLGC_WANTARROWS;
		}
	
	case WM_CHAR:
		{
			if (wParam == '\r' || wParam == 13 || wParam == VK_RETURN) {
				// Handle Enter key - insert a newline
				SendMessage(hWnd, EM_REPLACESEL, TRUE, (LPARAM)_T("\r\n"));
				return 0;
			}
			break;
		}
		
	case WM_KEYDOWN:
		if (wParam == VK_RETURN) {
			// Let WM_CHAR handle enter key processing
		}
		break;
		
	case WM_CTLCOLOREDIT:
		{
			// Set custom colors for the edit control
			HDC hdc = (HDC)wParam;
			SetTextColor(hdc, RGB(33, 37, 41));        // Dark gray text (modern)
			SetBkColor(hdc, RGB(255, 255, 255));       // Pure white background
			
			// Return the global brush for the background
			return (LRESULT)g_hEditBrush;
		}
	}
	
	// Call the original window procedure for all other messages
	return CallWindowProc(g_pOriginalEditProc, hWnd, message, wParam, lParam);
}

// “关于”框的消息处理程序。
INT_PTR CALLBACK About(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
	UNREFERENCED_PARAMETER(lParam);
	switch (message)
	{
	case WM_INITDIALOG:
		return (INT_PTR)TRUE;

	case WM_COMMAND:
		if (LOWORD(wParam) == IDOK || LOWORD(wParam) == IDCANCEL)
		{
			EndDialog(hDlg, LOWORD(wParam));
			return (INT_PTR)TRUE;
		}
		break;
	}
	return (INT_PTR)FALSE;
}
// Creation of main EditControl with enhanced appearance
VOID CreateEditControl(HWND &hwndEdit, HWND hWnd){
	hwndEdit = CreateWindowEx(
		WS_EX_CLIENTEDGE,   /* enhanced border style              */
		_T("EDIT"),         /* predefined class                  */
		NULL,               /* no window title                   */
		WS_CHILD | WS_VISIBLE | WS_HSCROLL | ES_MULTILINE |
		ES_LEFT | WS_VSCROLL | ES_WANTRETURN | ES_AUTOVSCROLL | ES_AUTOHSCROLL,
		0, 0, 300, 220,     /* set size in WM_SIZE message       */
		hWnd,               /* parent window                     */
		(HMENU)ID_EDIT,     /* edit control ID                   */
		(HINSTANCE)GetWindowLongPtr(hWnd, GWLP_HINSTANCE),
		NULL);              /* pointer not needed                */

	// Subclass the edit control to handle Enter key properly
	if (hwndEdit) {
		g_pOriginalEditProc = (WNDPROC)SetWindowLongPtr(hwndEdit, GWLP_WNDPROC, (LONG_PTR)EditControlSubclassProc);
		
		// Create a beautiful font if not already created
		if (!g_hEditFont) {
			g_hEditFont = CreateFont(
				16,                      // Height - slightly larger
				0,                       // Width (0 = default)
				0,                       // Escapement
				0,                       // Orientation
				FW_NORMAL,              // Weight
				FALSE,                  // Italic
				FALSE,                  // Underline
				FALSE,                  // Strikeout
				DEFAULT_CHARSET,        // Character set
				OUT_DEFAULT_PRECIS,     // Output precision
				CLIP_DEFAULT_PRECIS,    // Clipping precision
				CLEARTYPE_QUALITY,      // Quality
				DEFAULT_PITCH | FF_MODERN, // Pitch and family
				_T("Segoe UI")          // Modern font (fallback to system default if not available)
			);
		}
		
		// Create background brush if not already created
		if (!g_hEditBrush) {
			g_hEditBrush = CreateSolidBrush(RGB(255, 255, 255)); // Pure white background
		}
		
		// Set the font
		if (g_hEditFont) {
			SendMessage(hwndEdit, WM_SETFONT, (WPARAM)g_hEditFont, TRUE);
		}
		
		// Add generous margins for better readability
		SendMessage(hwndEdit, EM_SETMARGINS, EC_LEFTMARGIN | EC_RIGHTMARGIN, MAKELPARAM(12, 12));
		
		// Set tab stops for better formatting (every 4 characters)
		int tabStops = 32; // 4 characters * 8 units per character
		SendMessage(hwndEdit, EM_SETTABSTOPS, 1, (LPARAM)&tabStops);
		
		// Apply additional modern styling
		ApplyModernEditStyling(hwndEdit);
	}
}
VOID OpenDialogFileOpen(HWND hWnd, HWND hEdit){
	OPENFILENAME ofn;
	TCHAR szFile[MAX_PATH];

	//Initialize OPENFILENAME
	ZeroMemory(&ofn, sizeof(ofn));
	ofn.lStructSize = sizeof(ofn);
	ofn.hwndOwner = hWnd;
	ofn.lpstrFile = szFile;
	//Set lpstrfile[0] to \0 so that GetOpenFileName does not use the contents of szFile to initialize itself.
	ofn.lpstrFile[0] = _T('\0');
	ofn.nMaxFile = sizeof(szFile);
	ofn.nFilterIndex = 1;
	ofn.lpstrFilter = _T("All\0*.*\0Text\0*.TXT\0");
	ofn.lpstrFileTitle = NULL;
	ofn.lpstrInitialDir = NULL;
	ofn.Flags = OFN_PATHMUSTEXIST | OFN_FILEMUSTEXIST;

	//Display the Open dialog box
	if (GetOpenFileName(&ofn) == TRUE){
	
		DO_OPEN_FILE(hEdit, ofn.lpstrFile);
		// Store the opened file name for future save operations
		_tcscpy_s(g_szCurrentFileName, MAX_PATH, ofn.lpstrFile);
		g_bHasFileName = TRUE;
	}
	else{
		DWORD dwError = GetLastError();
		Error2Msgbox(dwError);
	}

}
VOID OpenDialogFileSaveAs(HWND hWnd, HWND hEdit){
	OPENFILENAME ofn;
	TCHAR szFile[MAX_PATH];

	//Initialize OPENFILENAME
	ZeroMemory(&ofn, sizeof(ofn));
	ofn.lStructSize = sizeof(ofn);
	ofn.hwndOwner = hWnd;
	ofn.lpstrFile = szFile;
	//Set lpstrfile[0] to \0 so that GetOpenFileName does not use the contents of szFile to initialize itself.
	ofn.lpstrFile[0] = '\0';
	ofn.nMaxFile = sizeof(szFile);
	ofn.nFilterIndex = 1;
	ofn.lpstrFilter = _T("All\0*.*\0Text\0*.TXT\0");
	ofn.lpstrFileTitle = NULL;
	ofn.lpstrInitialDir = NULL;
	ofn.Flags = OFN_PATHMUSTEXIST | OFN_OVERWRITEPROMPT |
		OFN_HIDEREADONLY;

	//Display the Save dialog box
	if (GetSaveFileName(&ofn) == TRUE){
		DO_SAVE_FILE(hEdit, ofn.lpstrFile);
		// Store the saved file name for future save operations
		_tcscpy_s(g_szCurrentFileName, MAX_PATH, ofn.lpstrFile);
		g_bHasFileName = TRUE;
	}
	else{
		DWORD dwError = GetLastError();
		Error2Msgbox(dwError);
	}

}

VOID OpenDialogFileSave(HWND hWnd, HWND hEdit){
	// If we already have a file name, save directly to that file
	if (g_bHasFileName && g_szCurrentFileName[0] != '\0') {
		DO_SAVE_FILE(hEdit, g_szCurrentFileName);
	}
	else {
		// No file name yet, open Save As dialog
		OpenDialogFileSaveAs(hWnd, hEdit);
	}
}

// Helper function to find text in edit control
BOOL FindTextInEdit(HWND hEdit, LPCTSTR findText, BOOL matchCase, BOOL wholeWord, BOOL findNext) {
	int textLen = GetWindowTextLength(hEdit);
	if (textLen == 0) return FALSE;

	// Get all text from edit control
	LPTSTR buffer = (LPTSTR)LocalAlloc(LMEM_ZEROINIT, (textLen + 1) * sizeof(TCHAR));
	if (!buffer) return FALSE;

	GetWindowText(hEdit, buffer, textLen + 1);

	// Get current selection
	DWORD selStart, selEnd;
	SendMessage(hEdit, EM_GETSEL, (WPARAM)&selStart, (LPARAM)&selEnd);

	// Determine search start position
	int startPos = findNext ? selEnd : (selStart > 0 ? selStart - 1 : 0);
	
	// Convert to lowercase for case-insensitive search if needed
	LPTSTR searchBuffer = buffer;
	LPTSTR searchText = (LPTSTR)findText;
	
	if (!matchCase) {
		CharLower(searchBuffer);
		searchText = (LPTSTR)LocalAlloc(LMEM_ZEROINIT, (_tcslen(findText) + 1) * sizeof(TCHAR));
		if (searchText) {
			_tcscpy_s(searchText, _tcslen(findText) + 1, findText);
			CharLower(searchText);
		}
	}

	// Search for text
	LPTSTR found = NULL;
	int foundPos = -1;

	if (findNext) {
		// Search forward
		if (startPos < textLen) {
			found = _tcsstr(searchBuffer + startPos, searchText ? searchText : findText);
			if (found) {
				foundPos = found - searchBuffer;
			}
		}
		// If not found and not at beginning, search from start
		if (!found && startPos > 0) {
			found = _tcsstr(searchBuffer, searchText ? searchText : findText);
			if (found) {
				foundPos = found - searchBuffer;
			}
		}
	} else {
		// Search backward - simple implementation
		for (int i = startPos; i >= 0; i--) {
			if (_tcsnicmp(searchBuffer + i, searchText ? searchText : findText, _tcslen(findText)) == 0) {
				if (!matchCase || _tcsncmp(buffer + i, findText, _tcslen(findText)) == 0) {
					foundPos = i;
					break;
				}
			}
		}
	}

	// Clean up
	LocalFree(buffer);
	if (searchText && searchText != findText) {
		LocalFree(searchText);
	}

	// Select found text
	if (foundPos >= 0) {
		// IMPORTANT: Set focus to edit control AND bring parent window to front
		SetFocus(hEdit);
		SetForegroundWindow(GetParent(hEdit));
		
		// Set selection using proper casting
		SendMessage(hEdit, EM_SETSEL, (WPARAM)foundPos, (LPARAM)(foundPos + _tcslen(findText)));
		
		// Scroll to make sure the selection is visible
		SendMessage(hEdit, EM_SCROLLCARET, 0, 0);
		
		// Force a repaint to ensure the selection is visible
		InvalidateRect(hEdit, NULL, TRUE);
		UpdateWindow(hEdit);
		
		return TRUE;
	}

	return FALSE;
}

// Helper function to replace text in edit control
BOOL ReplaceTextInEdit(HWND hEdit, LPCTSTR findText, LPCTSTR replaceText, BOOL matchCase, BOOL wholeWord, BOOL replaceAll) {
	int replacedCount = 0;
	DWORD selStart, selEnd;
	
	if (replaceAll) {
		// For replace all, start from the beginning
		SendMessage(hEdit, EM_SETSEL, 0, 0);
		
		while (FindTextInEdit(hEdit, findText, matchCase, wholeWord, TRUE)) {
			// Get current selection
			SendMessage(hEdit, EM_GETSEL, (WPARAM)&selStart, (LPARAM)&selEnd);
			
			// Replace the selected text
			SendMessage(hEdit, EM_REPLACESEL, TRUE, (LPARAM)replaceText);
			
			// Move cursor to after the replaced text to continue searching
			int newPos = selStart + _tcslen(replaceText);
			SendMessage(hEdit, EM_SETSEL, newPos, newPos);
			
			replacedCount++;
			
			// Prevent infinite loop for empty search strings
			if (_tcslen(findText) == 0) break;
		}
		
		return replacedCount > 0;
	} else {
		// For single replace, check if current selection matches the find text
		SendMessage(hEdit, EM_GETSEL, (WPARAM)&selStart, (LPARAM)&selEnd);
		
		if (selStart != selEnd) {
			// Get selected text
			int selLength = selEnd - selStart;
			LPTSTR selectedText = (LPTSTR)LocalAlloc(LMEM_ZEROINIT, (selLength + 1) * sizeof(TCHAR));
			if (selectedText) {
				// Get all text first
				int textLen = GetWindowTextLength(hEdit);
				LPTSTR allText = (LPTSTR)LocalAlloc(LMEM_ZEROINIT, (textLen + 1) * sizeof(TCHAR));
				if (allText) {
					GetWindowText(hEdit, allText, textLen + 1);
					// Copy the selected portion
					_tcsncpy_s(selectedText, selLength + 1, allText + selStart, selLength);
					LocalFree(allText);
					
					// Check if selected text matches find text
					BOOL matches = FALSE;
					if (matchCase) {
						matches = (_tcscmp(selectedText, findText) == 0);
					} else {
						matches = (_tcsicmp(selectedText, findText) == 0);
					}
					
					if (matches) {
						// Replace the selected text
						SendMessage(hEdit, EM_REPLACESEL, TRUE, (LPARAM)replaceText);
						LocalFree(selectedText);
						return TRUE;
					}
				}
				
				LocalFree(selectedText);
			}
		}
		
		// If no matching selection, find the next occurrence
		return FindTextInEdit(hEdit, findText, matchCase, wholeWord, TRUE);
	}
}

// Find Dialog Procedure
INT_PTR CALLBACK FindDialog(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam) {
	UNREFERENCED_PARAMETER(lParam);
	
	switch (message) {
	case WM_INITDIALOG:
		// Set focus to the find text box
		SetFocus(GetDlgItem(hDlg, IDC_FIND_TEXT));
		// Set previous search text if any
		if (g_szFindText[0] != '\0') {
			SetDlgItemText(hDlg, IDC_FIND_TEXT, g_szFindText);
		}
		// Set checkbox states
		CheckDlgButton(hDlg, IDC_FIND_MATCH_CASE, g_bMatchCase ? BST_CHECKED : BST_UNCHECKED);
		CheckDlgButton(hDlg, IDC_FIND_WHOLE_WORD, g_bWholeWord ? BST_CHECKED : BST_UNCHECKED);
		return (INT_PTR)FALSE; // Return FALSE since we set focus manually

	case WM_COMMAND:
		switch (LOWORD(wParam)) {
		case IDC_FIND_NEXT:
		case IDC_FIND_PREVIOUS:
			{
				// Get search text
				GetDlgItemText(hDlg, IDC_FIND_TEXT, g_szFindText, 255);
				if (_tcslen(g_szFindText) == 0) {
					MessageBox(hDlg, _T("Please enter text to find."), _T("Find"), MB_OK | MB_ICONINFORMATION);
					return (INT_PTR)TRUE;
				}

				// Get checkbox states
				g_bMatchCase = IsDlgButtonChecked(hDlg, IDC_FIND_MATCH_CASE) == BST_CHECKED;
				g_bWholeWord = IsDlgButtonChecked(hDlg, IDC_FIND_WHOLE_WORD) == BST_CHECKED;

				// Perform search
				BOOL findNext = (LOWORD(wParam) == IDC_FIND_NEXT);
				if (!FindTextInEdit(g_hEditControl, g_szFindText, g_bMatchCase, g_bWholeWord, findNext)) {
					MessageBox(hDlg, _T("Cannot find the specified text."), _T("Find"), MB_OK | MB_ICONINFORMATION);
				}
			}
			return (INT_PTR)TRUE;

		case IDC_FIND_CLOSE:
		case IDCANCEL:
			g_hFindDialog = NULL;
			EndDialog(hDlg, LOWORD(wParam));
			return (INT_PTR)TRUE;
		}
		break;

	case WM_CLOSE:
		g_hFindDialog = NULL;
		EndDialog(hDlg, 0);
		return (INT_PTR)TRUE;
	}
	
	return (INT_PTR)FALSE;
}

// Show Find Dialog
VOID ShowFindDialog(HWND hWnd, HWND hEdit) {
	g_hEditControl = hEdit;
	
	// If dialog is already open, just bring it to front
	if (g_hFindDialog && IsWindow(g_hFindDialog)) {
		SetForegroundWindow(g_hFindDialog);
		return;
	}

	// Create and show the dialog as a modeless dialog
	g_hFindDialog = CreateDialog(hInst, MAKEINTRESOURCE(IDD_FIND_DIALOG), hWnd, FindDialog);
	if (g_hFindDialog) {
		// Show dialog but don't steal focus from edit control
		ShowWindow(g_hFindDialog, SW_SHOW);
		// Ensure edit control keeps focus
		SetFocus(hEdit);
	}
}

// Find Next function - uses the last search parameters
VOID FindNext(HWND hEdit) {
	// Check if there's a previous search text
	if (_tcslen(g_szFindText) == 0) {
		// No previous search text, show find dialog
		ShowFindDialog(GetParent(hEdit), hEdit);
		return;
	}

	// Use the stored search parameters to find the next occurrence
	if (!FindTextInEdit(hEdit, g_szFindText, g_bMatchCase, g_bWholeWord, TRUE)) {
		// If not found, show message
		MessageBox(GetParent(hEdit), _T("Cannot find the specified text."), _T("Find Next"), MB_OK | MB_ICONINFORMATION);
	}
}

// Replace Dialog Procedure
INT_PTR CALLBACK ReplaceDialog(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam) {
	UNREFERENCED_PARAMETER(lParam);
	
	switch (message) {
	case WM_INITDIALOG:
		// Set focus to the find text box
		SetFocus(GetDlgItem(hDlg, IDC_REPLACE_FIND_TEXT));
		// Set previous search text if any
		if (g_szFindText[0] != '\0') {
			SetDlgItemText(hDlg, IDC_REPLACE_FIND_TEXT, g_szFindText);
		}
		if (g_szReplaceText[0] != '\0') {
			SetDlgItemText(hDlg, IDC_REPLACE_WITH_TEXT, g_szReplaceText);
		}
		// Set checkbox states
		CheckDlgButton(hDlg, IDC_REPLACE_MATCH_CASE, g_bMatchCase ? BST_CHECKED : BST_UNCHECKED);
		CheckDlgButton(hDlg, IDC_REPLACE_WHOLE_WORD, g_bWholeWord ? BST_CHECKED : BST_UNCHECKED);
		return (INT_PTR)FALSE; // Return FALSE since we set focus manually

	case WM_COMMAND:
		switch (LOWORD(wParam)) {
		case IDC_REPLACE_FIND_NEXT:
			{
				// Get search text
				GetDlgItemText(hDlg, IDC_REPLACE_FIND_TEXT, g_szFindText, 255);
				if (_tcslen(g_szFindText) == 0) {
					MessageBox(hDlg, _T("Please enter text to find."), _T("Replace"), MB_OK | MB_ICONINFORMATION);
					return (INT_PTR)TRUE;
				}

				// Get checkbox states
				g_bMatchCase = IsDlgButtonChecked(hDlg, IDC_REPLACE_MATCH_CASE) == BST_CHECKED;
				g_bWholeWord = IsDlgButtonChecked(hDlg, IDC_REPLACE_WHOLE_WORD) == BST_CHECKED;

				// Perform search
				if (!FindTextInEdit(g_hEditControl, g_szFindText, g_bMatchCase, g_bWholeWord, TRUE)) {
					MessageBox(hDlg, _T("Cannot find the specified text."), _T("Replace"), MB_OK | MB_ICONINFORMATION);
				}
			}
			return (INT_PTR)TRUE;

		case IDC_REPLACE_REPLACE:
			{
				// Get search and replace text
				GetDlgItemText(hDlg, IDC_REPLACE_FIND_TEXT, g_szFindText, 255);
				GetDlgItemText(hDlg, IDC_REPLACE_WITH_TEXT, g_szReplaceText, 255);
				
				if (_tcslen(g_szFindText) == 0) {
					MessageBox(hDlg, _T("Please enter text to find."), _T("Replace"), MB_OK | MB_ICONINFORMATION);
					return (INT_PTR)TRUE;
				}

				// Get checkbox states
				g_bMatchCase = IsDlgButtonChecked(hDlg, IDC_REPLACE_MATCH_CASE) == BST_CHECKED;
				g_bWholeWord = IsDlgButtonChecked(hDlg, IDC_REPLACE_WHOLE_WORD) == BST_CHECKED;

				// Perform replace
				if (!ReplaceTextInEdit(g_hEditControl, g_szFindText, g_szReplaceText, g_bMatchCase, g_bWholeWord, FALSE)) {
					MessageBox(hDlg, _T("Cannot find the specified text."), _T("Replace"), MB_OK | MB_ICONINFORMATION);
				}
			}
			return (INT_PTR)TRUE;

		case IDC_REPLACE_REPLACE_ALL:
			{
				// Get search and replace text
				GetDlgItemText(hDlg, IDC_REPLACE_FIND_TEXT, g_szFindText, 255);
				GetDlgItemText(hDlg, IDC_REPLACE_WITH_TEXT, g_szReplaceText, 255);
				
				if (_tcslen(g_szFindText) == 0) {
					MessageBox(hDlg, _T("Please enter text to find."), _T("Replace"), MB_OK | MB_ICONINFORMATION);
					return (INT_PTR)TRUE;
				}

				// Get checkbox states
				g_bMatchCase = IsDlgButtonChecked(hDlg, IDC_REPLACE_MATCH_CASE) == BST_CHECKED;
				g_bWholeWord = IsDlgButtonChecked(hDlg, IDC_REPLACE_WHOLE_WORD) == BST_CHECKED;

				// Perform replace all
				if (ReplaceTextInEdit(g_hEditControl, g_szFindText, g_szReplaceText, g_bMatchCase, g_bWholeWord, TRUE)) {
					MessageBox(hDlg, _T("Replace operation completed."), _T("Replace"), MB_OK | MB_ICONINFORMATION);
				} else {
					MessageBox(hDlg, _T("Cannot find the specified text."), _T("Replace"), MB_OK | MB_ICONINFORMATION);
				}
			}
			return (INT_PTR)TRUE;

		case IDC_REPLACE_CLOSE:
		case IDCANCEL:
			g_hReplaceDialog = NULL;
			EndDialog(hDlg, LOWORD(wParam));
			return (INT_PTR)TRUE;
		}
		break;

	case WM_CLOSE:
		g_hReplaceDialog = NULL;
		EndDialog(hDlg, 0);
		return (INT_PTR)TRUE;
	}
	
	return (INT_PTR)FALSE;
}

// Show Replace Dialog
VOID ShowReplaceDialog(HWND hWnd, HWND hEdit) {
	g_hEditControl = hEdit;
	
	// If dialog is already open, just bring it to front
	if (g_hReplaceDialog && IsWindow(g_hReplaceDialog)) {
		SetForegroundWindow(g_hReplaceDialog);
		return;
	}

	// Create and show the dialog as a modeless dialog
	g_hReplaceDialog = CreateDialog(hInst, MAKEINTRESOURCE(IDD_REPLACE_DIALOG), hWnd, ReplaceDialog);
	if (g_hReplaceDialog) {
		// Show dialog but don't steal focus from edit control
		ShowWindow(g_hReplaceDialog, SW_SHOW);
		// Ensure edit control keeps focus
		SetFocus(hEdit);
	}
}

// Enhanced styling function for edit control
VOID ApplyModernEditStyling(HWND hwndEdit) {
	if (!hwndEdit) return;
	
	// Enable undo functionality (available in newer Windows versions)
	#ifdef EM_SETUNDOLIMIT
	SendMessage(hwndEdit, EM_SETUNDOLIMIT, 100, 0);  // Set undo limit
	#endif
	
	// Set formatting rectangle to add internal padding
	RECT rect;
	GetClientRect(hwndEdit, &rect);
	rect.left += 4;
	rect.top += 4;
	rect.right -= 4;
	rect.bottom -= 4;
	SendMessage(hwndEdit, EM_SETRECT, 0, (LPARAM)&rect);
}

// Cleanup function for edit control resources
VOID CleanupEditResources() {
	if (g_hEditFont) {
		DeleteObject(g_hEditFont);
		g_hEditFont = NULL;
	}
	
	if (g_hEditBrush) {
		DeleteObject(g_hEditBrush);
		g_hEditBrush = NULL;
	}
}

// Simple test function to verify selection works
VOID TestSelection(HWND hEdit) {
	// Test if basic selection works
	SetFocus(hEdit);
	SendMessage(hEdit, EM_SETSEL, 0, 5); // Select first 5 characters
	InvalidateRect(hEdit, NULL, TRUE);
	UpdateWindow(hEdit);
}

// 文本选择相关函数实现
VOID SelectAllText(HWND hEdit) {
	if (!hEdit) return;
	
	// 选择所有文本 (0 到 -1 表示选择全部)
	SendMessage(hEdit, EM_SETSEL, 0, -1);
	
	// 确保编辑控件有焦点
	SetFocus(hEdit);
}

VOID TestSelectAllFunction(HWND hEdit) {
	if (!hEdit) return;
	
	// 设置一些测试文本
	SetWindowText(hEdit, _T("这是测试文本\r\n第二行\r\n第三行\r\n用于测试全选功能"));
	
	// 延迟一下，然后执行全选
	Sleep(500);
	SelectAllText(hEdit);
}

// 初始化打印设置
VOID InitializePrintSettings() {
	if (g_bPrintInitialized) return;
	
	// 初始化打印对话框结构
	ZeroMemory(&g_printDlg, sizeof(g_printDlg));
	g_printDlg.lStructSize = sizeof(g_printDlg);
	g_printDlg.Flags = PD_RETURNDC | PD_ALLPAGES | PD_USEDEVMODECOPIES | PD_NOPAGENUMS | PD_HIDEPRINTTOFILE | PD_NOSELECTION;
	
	// 初始化页面设置对话框结构
	ZeroMemory(&g_pageSetupDlg, sizeof(g_pageSetupDlg));
	g_pageSetupDlg.lStructSize = sizeof(g_pageSetupDlg);
	g_pageSetupDlg.Flags = PSD_MARGINS | PSD_INTHOUSANDTHSOFINCHES;
	g_pageSetupDlg.rtMargin.left = 1000;    // 1 inch
	g_pageSetupDlg.rtMargin.top = 1000;     // 1 inch
	g_pageSetupDlg.rtMargin.right = 1000;   // 1 inch
	g_pageSetupDlg.rtMargin.bottom = 1000;  // 1 inch
	
	g_bPrintInitialized = TRUE;
}

// 显示打印对话框并执行打印
VOID ShowPrintDialog(HWND hWnd, HWND hEdit) {
	InitializePrintSettings();
	
	// 检查是否有文本内容
	int textLength = GetWindowTextLength(hEdit);
	if (textLength == 0) {
		MessageBox(hWnd, _T("No text to print."), _T("Print"), MB_OK | MB_ICONINFORMATION);
		return;
	}
	
	g_printDlg.hwndOwner = hWnd;
	
	// 显示打印对话框
	if (PrintDlg(&g_printDlg)) {
		HDC hPrinterDC = g_printDlg.hDC;
		
		if (hPrinterDC) {
			// 执行打印
			if (!PrintTextDocument(hWnd, hEdit, hPrinterDC)) {
				MessageBox(hWnd, _T("Print failed."), _T("Print Error"), MB_OK | MB_ICONERROR);
			}
			
			// 清理打印机DC
			DeleteDC(hPrinterDC);
		}
	}
}

// 显示页面设置对话框
VOID ShowPageSetupDialog(HWND hWnd) {
	InitializePrintSettings();
	
	g_pageSetupDlg.hwndOwner = hWnd;
	
	// 显示页面设置对话框
	if (PageSetupDlg(&g_pageSetupDlg)) {
		// 页面设置已更新
		MessageBox(hWnd, _T("Page setup updated."), _T("Page Setup"), MB_OK | MB_ICONINFORMATION);
	}
}

// 计算文本度量和页面布局
VOID CalculateTextMetrics(HDC hDC, TEXTMETRIC* tm, int* linesPerPage, int* printableWidth) {
	// 先设置打印字体
	HFONT hPrintFont = CreateFont(
		-MulDiv(14, GetDeviceCaps(hDC, LOGPIXELSY), 72), // 14磅字体，按打印机DPI缩放
		0,                       // 宽度自动
		0,                       // 不旋转
		0,                       // 不倾斜
		FW_NORMAL,              // 正常粗细
		FALSE,                  // 不斜体
		FALSE,                  // 不下划线
		FALSE,                  // 不删除线
		DEFAULT_CHARSET,        // 默认字符集
		OUT_DEFAULT_PRECIS,     // 输出精度
		CLIP_DEFAULT_PRECIS,    // 裁剪精度
		CLEARTYPE_QUALITY,      // 高质量
		DEFAULT_PITCH | FF_MODERN, // 字体族
		_T("Segoe UI")          // 字体名称
	);
	
	HFONT hOldFont = NULL;
	if (hPrintFont) {
		hOldFont = (HFONT)SelectObject(hDC, hPrintFont);
	}
	
	GetTextMetrics(hDC, tm);
	
	// 恢复原字体
	if (hOldFont) {
		SelectObject(hDC, hOldFont);
	}
	if (hPrintFont) {
		DeleteObject(hPrintFont);
	}
	
	int pageWidth = GetDeviceCaps(hDC, HORZRES);
	int pageHeight = GetDeviceCaps(hDC, VERTRES);
	
	// 按打印机DPI计算页边距（约0.75英寸）
	int dpiX = GetDeviceCaps(hDC, LOGPIXELSX);
	int dpiY = GetDeviceCaps(hDC, LOGPIXELSY);
	int leftMargin = MulDiv(750, dpiX, 1000);   // 0.75英寸
	int topMargin = MulDiv(750, dpiY, 1000);    // 0.75英寸
	int rightMargin = leftMargin;
	int bottomMargin = topMargin;
	
	*printableWidth = pageWidth - leftMargin - rightMargin;
	int printableHeight = pageHeight - topMargin - bottomMargin;
	
	// 使用实际行高计算每页行数
	*linesPerPage = printableHeight / tm->tmHeight;
	
	// 确保合理的最小值
	if (*printableWidth < 200) *printableWidth = 200;
	if (*linesPerPage < 5) *linesPerPage = 5;
}

// 计算文本行在指定宽度内能容纳的字符数
int CalculateLineLength(HDC hDC, LPCTSTR text, int maxWidth) {
	if (!text || maxWidth <= 0) return 0;
	
	int textLen = _tcslen(text);
	SIZE textSize;
	
	// 使用二分查找找到最大能容纳的字符数
	int left = 0, right = textLen;
	int result = 0;
	
	while (left <= right) {
		int mid = (left + right) / 2;
		
		if (GetTextExtentPoint32(hDC, text, mid, &textSize)) {
			if (textSize.cx <= maxWidth) {
				result = mid;
				left = mid + 1;
			} else {
				right = mid - 1;
			}
		} else {
			break;
		}
	}
	
	return result;
}

// 打印文本页面
BOOL PrintTextPage(HDC hDC, HWND hEdit, int startLine, int endLine, int pageNumber) {
	TEXTMETRIC tm;
	int linesPerPage, printableWidth;
	CalculateTextMetrics(hDC, &tm, &linesPerPage, &printableWidth);
	
	// 获取编辑控件中的所有文本
	int totalTextLength = GetWindowTextLength(hEdit);
	if (totalTextLength == 0) return FALSE;
	
	LPTSTR allText = (LPTSTR)LocalAlloc(LMEM_ZEROINIT, (totalTextLength + 1) * sizeof(TCHAR));
	if (!allText) return FALSE;
	
	GetWindowText(hEdit, allText, totalTextLength + 1);
	
	// 分析文本，按行分割
	LPTSTR* displayLines = (LPTSTR*)LocalAlloc(LMEM_ZEROINIT, linesPerPage * sizeof(LPTSTR));
	if (!displayLines) {
		LocalFree(allText);
		return FALSE;
	}
	
	int currentOriginalLine = 0;
	int displayLineCount = 0;
	LPTSTR lineStart = allText;
	LPTSTR textPos = allText;
	
	// 跳过到起始行
	while (currentOriginalLine < startLine && *textPos) {
		if (*textPos == _T('\r') && *(textPos + 1) == _T('\n')) {
			currentOriginalLine++;
			textPos += 2;
			lineStart = textPos;
		} else if (*textPos == _T('\n')) {
			currentOriginalLine++;
			textPos++;
			lineStart = textPos;
		} else {
			textPos++;
		}
	}
	
	// 处理要打印的行，考虑自动换行
	while (currentOriginalLine <= endLine && displayLineCount < linesPerPage && *textPos) {
		// 找到当前行的结束位置
		LPTSTR lineEnd = textPos;
		while (*lineEnd && *lineEnd != _T('\r') && *lineEnd != _T('\n')) {
			lineEnd++;
		}
		
		// 计算当前原始行的长度
		int originalLineLength = lineEnd - lineStart;
		
		if (originalLineLength == 0) {
			// 空行
			displayLines[displayLineCount] = (LPTSTR)LocalAlloc(LMEM_ZEROINIT, sizeof(TCHAR));
			if (displayLines[displayLineCount]) {
				displayLines[displayLineCount][0] = _T('\0');
				displayLineCount++;
			}
		} else {
			// 非空行，可能需要分成多个显示行
			LPTSTR currentPos = lineStart;
			
			while (currentPos < lineEnd && displayLineCount < linesPerPage) {
				// 计算在当前宽度下能容纳多少字符
				int remainingChars = lineEnd - currentPos;
				int charsInThisLine = CalculateLineLength(hDC, currentPos, printableWidth);
				
				if (charsInThisLine == 0 && remainingChars > 0) {
					// 至少要显示一个字符，即使超宽
					charsInThisLine = 1;
				}
				
				if (charsInThisLine > remainingChars) {
					charsInThisLine = remainingChars;
				}
				
				// 分配并复制这一显示行的文本
				displayLines[displayLineCount] = (LPTSTR)LocalAlloc(LMEM_ZEROINIT, (charsInThisLine + 1) * sizeof(TCHAR));
				if (displayLines[displayLineCount]) {
					_tcsncpy_s(displayLines[displayLineCount], charsInThisLine + 1, currentPos, charsInThisLine);
					displayLineCount++;
					currentPos += charsInThisLine;
				} else {
					break;
				}
			}
		}
		
		// 移动到下一个原始行
		currentOriginalLine++;
		if (*lineEnd == _T('\r') && *(lineEnd + 1) == _T('\n')) {
			textPos = lineEnd + 2;
		} else if (*lineEnd == _T('\n')) {
			textPos = lineEnd + 1;
		} else {
			break;
		}
		lineStart = textPos;
	}
	
	// 设置打印字体（确保与度量计算一致）
	HFONT hPrintFont = CreateFont(
		-MulDiv(14, GetDeviceCaps(hDC, LOGPIXELSY), 72), // 14磅字体，按打印机DPI缩放
		0,                       // 宽度自动
		0,                       // 不旋转
		0,                       // 不倾斜
		FW_NORMAL,              // 正常粗细
		FALSE,                  // 不斜体
		FALSE,                  // 不下划线
		FALSE,                  // 不删除线
		DEFAULT_CHARSET,        // 默认字符集
		OUT_DEFAULT_PRECIS,     // 输出精度
		CLIP_DEFAULT_PRECIS,    // 裁剪精度
		CLEARTYPE_QUALITY,      // 高质量
		DEFAULT_PITCH | FF_MODERN, // 字体族
		_T("Segoe UI")          // 字体名称
	);
	
	HFONT hOldFont = NULL;
	if (hPrintFont) {
		hOldFont = (HFONT)SelectObject(hDC, hPrintFont);
	}
	
	// 重新获取文本度量以确保一致性
	TEXTMETRIC actualTm;
	GetTextMetrics(hDC, &actualTm);
	
	// 计算页边距
	int dpiX = GetDeviceCaps(hDC, LOGPIXELSX);
	int dpiY = GetDeviceCaps(hDC, LOGPIXELSY);
	int leftMargin = MulDiv(750, dpiX, 1000);   // 0.75英寸
	int topMargin = MulDiv(750, dpiY, 1000);    // 0.75英寸
	
	// 打印页面内容
	int yPos = topMargin; // 从正确的顶部边距开始
	
	// 打印页眉（页码）
	TCHAR pageHeader[64];
	_stprintf_s(pageHeader, 64, _T("Page %d"), pageNumber);
	TextOut(hDC, GetDeviceCaps(hDC, HORZRES) - MulDiv(1500, dpiX, 1000), MulDiv(500, dpiY, 1000), pageHeader, _tcslen(pageHeader));
	
	// 打印文本行，使用实际的行高
	for (int i = 0; i < displayLineCount; i++) {
		if (displayLines[i]) {
			TextOut(hDC, leftMargin, yPos, displayLines[i], _tcslen(displayLines[i]));
			yPos += actualTm.tmHeight; // 使用实际的行高，没有额外间距
			LocalFree(displayLines[i]);
		}
	}
	
	// 恢复原字体并清理
	if (hOldFont) {
		SelectObject(hDC, hOldFont);
	}
	if (hPrintFont) {
		DeleteObject(hPrintFont);
	}
	
	// 清理内存
	LocalFree(displayLines);
	LocalFree(allText);
	
	return TRUE;
}

// 打印整个文档
BOOL PrintTextDocument(HWND hWnd, HWND hEdit, HDC hPrinterDC) {
	DOCINFO docInfo;
	ZeroMemory(&docInfo, sizeof(docInfo));
	docInfo.cbSize = sizeof(docInfo);
	docInfo.lpszDocName = _T("MyNotePad Document");
	
	// 开始打印作业
	if (StartDoc(hPrinterDC, &docInfo) <= 0) {
		return FALSE;
	}
	
	// 计算总页数
	TEXTMETRIC tm;
	int linesPerPage, printableWidth;
	CalculateTextMetrics(hPrinterDC, &tm, &linesPerPage, &printableWidth);
	
	// 计算总行数
	int totalLines = SendMessage(hEdit, EM_GETLINECOUNT, 0, 0);
	int totalPages = (totalLines + linesPerPage - 1) / linesPerPage;
	
	if (totalPages == 0) totalPages = 1;
	
	// 打印每一页
	for (int page = 1; page <= totalPages; page++) {
		// 开始新页
		if (StartPage(hPrinterDC) <= 0) {
			EndDoc(hPrinterDC);
			return FALSE;
		}
		
		// 计算这一页的行范围
		int startLine = (page - 1) * linesPerPage;
		int endLine = min(startLine + linesPerPage - 1, totalLines - 1);
		
		// 打印这一页
		if (!PrintTextPage(hPrinterDC, hEdit, startLine, endLine, page)) {
			EndPage(hPrinterDC);
			EndDoc(hPrinterDC);
			return FALSE;
		}
		
		// 结束页面
		if (EndPage(hPrinterDC) <= 0) {
			EndDoc(hPrinterDC);
			return FALSE;
		}
	}
	
	// 结束打印作业
	EndDoc(hPrinterDC);
	
	MessageBox(hWnd, _T("Document printed successfully."), _T("Print Complete"), MB_OK | MB_ICONINFORMATION);
	return TRUE;
}