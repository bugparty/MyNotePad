﻿#include "stdafx.h"
#include "Dialog.h"
#include "resource.h"
#include "Utils.h"
#include "Document.h"
#include "Registry.h"
#include "Print.h"
#include "StatusBar.h"

// External global variables
extern HINSTANCE hInst;



// Global variables for Find functionality
static HWND g_hFindDialog = NULL;
static HWND g_hReplaceDialog = NULL;
static HWND g_hGoToDialog = NULL;
static HWND g_hEditControl = NULL;
static TCHAR g_szFindText[256] = {0};
static TCHAR g_szReplaceText[256] = {0};
static BOOL g_bMatchCase = FALSE;
static BOOL g_bWholeWord = FALSE;

// Font related global variables
static LOGFONT g_currentLogFont = {0};
static BOOL g_bFontInitialized = FALSE;

// Word Wrap related global variables
static BOOL g_bWordWrapEnabled = FALSE;

// Original edit control window procedure
static WNDPROC g_pOriginalEditProc = NULL;

// Global resources for edit control styling
static HFONT g_hEditFont = NULL;
static HBRUSH g_hEditBrush = NULL;

// Subclassed edit control window procedure
LRESULT CALLBACK EditControlSubclassProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam) {
	switch (message) {
	case WM_GETDLGCODE:
		{
			// Tell the system we want to handle specific keys, but allow accelerators to pass through
			// Don't use DLGC_WANTALLKEYS as it blocks accelerator keys like Ctrl+V, Ctrl+C, etc.
			return DLGC_HASSETSEL | DLGC_WANTCHARS | DLGC_WANTARROWS;
		}
	
	case WM_CHAR:
		{
			if (wParam == '\r' || wParam == 13 || wParam == VK_RETURN) {
				// Handle Enter key - insert a newline
				SendMessage(hWnd, EM_REPLACESEL, TRUE, (LPARAM)_T("\r\n"));
				// Update cursor position after text change
				UpdateCursorPosition(hWnd);
				return 0;
			}
			break;
		}
		
	case WM_KEYDOWN:
		// Handle special accelerator keys - send commands directly to main window
		if ((GetKeyState(VK_CONTROL) & 0x8000)) {
			// Ctrl key is pressed - handle common accelerator keys
			switch (wParam) {
			case 'V':  // Ctrl+V (Paste)
				PostMessage(GetParent(hWnd), WM_COMMAND, IDM_EDIT_PASTE, 0);
				return 0;
			case 'C':  // Ctrl+C (Copy)
				PostMessage(GetParent(hWnd), WM_COMMAND, IDM_EDIT_COPY, 0);
				return 0;
			case 'X':  // Ctrl+X (Cut)
				PostMessage(GetParent(hWnd), WM_COMMAND, IDM_EDIT_CUT, 0);
				return 0;
			case 'A':  // Ctrl+A (Select All)
				PostMessage(GetParent(hWnd), WM_COMMAND, IDM_EDIT_SELECTALL, 0);
				return 0;
			case 'Z':  // Ctrl+Z (Undo)
				PostMessage(GetParent(hWnd), WM_COMMAND, IDM_EDIT_UNDO, 0);
				return 0;
			case 'Y':  // Ctrl+Y (Redo)
				PostMessage(GetParent(hWnd), WM_COMMAND, IDM_EDIT_REDO, 0);
				return 0;
			case 'F':  // Ctrl+F (Find)
				PostMessage(GetParent(hWnd), WM_COMMAND, IDM_EDIT_FIND, 0);
				return 0;
			case 'H':  // Ctrl+H (Replace)
				PostMessage(GetParent(hWnd), WM_COMMAND, IDM_EDIT_REPLACE, 0);
				return 0;
			case 'G':  // Ctrl+G (Go To)
				PostMessage(GetParent(hWnd), WM_COMMAND, IDM_EDIT_GOTO, 0);
				return 0;
			case 'N':  // Ctrl+N (New)
				PostMessage(GetParent(hWnd), WM_COMMAND, IDM_FILE_NEW, 0);
				return 0;
			case 'O':  // Ctrl+O (Open)
				PostMessage(GetParent(hWnd), WM_COMMAND, IDM_FILE_OPEN, 0);
				return 0;
			case 'S':  // Ctrl+S (Save)
				// Check if Shift is also pressed for Save As
				if (GetKeyState(VK_SHIFT) & 0x8000) {
					PostMessage(GetParent(hWnd), WM_COMMAND, IDM_FILE_SAVEAS, 0);
				} else {
					PostMessage(GetParent(hWnd), WM_COMMAND, IDM_FILE_SAVE, 0);
				}
				return 0;
			case 'P':  // Ctrl+P (Print)
				PostMessage(GetParent(hWnd), WM_COMMAND, IDM_FILE_PRINT, 0);
				return 0;
			}
		}
		
		// Handle navigation keys and update cursor position
		if (wParam == VK_RETURN) {
			// Let WM_CHAR handle enter key processing
		} else if (wParam == VK_UP || wParam == VK_DOWN || wParam == VK_LEFT || wParam == VK_RIGHT ||
				   wParam == VK_HOME || wParam == VK_END || wParam == VK_PRIOR || wParam == VK_NEXT) {
			// Call original procedure first, then update position
			LRESULT result = CallWindowProc(g_pOriginalEditProc, hWnd, message, wParam, lParam);
			UpdateCursorPosition(hWnd);
			return result;
		}
		break;
		
	case WM_LBUTTONUP:
		{
			// Update cursor position after mouse click
			LRESULT result = CallWindowProc(g_pOriginalEditProc, hWnd, message, wParam, lParam);
			UpdateCursorPosition(hWnd);
			return result;
		}
		
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
	LRESULT result = CallWindowProc(g_pOriginalEditProc, hWnd, message, wParam, lParam);
	
	// Update cursor position for text modification messages
	if (message == WM_CHAR || message == WM_PASTE || message == WM_CUT || message == WM_CLEAR) {
		UpdateCursorPosition(hWnd);
	}
	
	return result;
}

// Creation of main EditControl with enhanced appearance
VOID CreateEditControl(HWND &hwndEdit, HWND hWnd){
	// Set edit control style based on Word Wrap status
	DWORD dwStyle = WS_CHILD | WS_VISIBLE | WS_VSCROLL | ES_MULTILINE |
		ES_LEFT | ES_WANTRETURN | ES_AUTOVSCROLL;
	
	if (!g_bWordWrapEnabled) {
		// If Word Wrap is not enabled, add horizontal scroll bar
		dwStyle |= WS_HSCROLL | ES_AUTOHSCROLL;
	}
	
	hwndEdit = CreateWindowEx(
		WS_EX_CLIENTEDGE,   /* enhanced border style              */
		_T("EDIT"),         /* predefined class                  */
		NULL,               /* no window title                   */
		dwStyle,
		0, 0, 300, 220,     /* set size in WM_SIZE message       */
		hWnd,               /* parent window                     */
		(HMENU)ID_EDIT,     /* edit control ID                   */
		(HINSTANCE)GetWindowLongPtr(hWnd, GWLP_HINSTANCE),
		NULL);              /* pointer not needed                */

	// Subclass the edit control for custom keyboard handling and appearance
	if (hwndEdit) {
		g_pOriginalEditProc = (WNDPROC)SetWindowLongPtr(hwndEdit, GWLP_WNDPROC, (LONG_PTR)EditControlSubclassProc);
		
		// Initialize default font settings
		InitializeDefaultFont();
		
		// Create font from LOGFONT if not already created
		if (!g_hEditFont) {
			g_hEditFont = CreateFontIndirect(&g_currentLogFont);
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

// Helper function to go to a specific line in edit control
BOOL GoToLine(HWND hEdit, int lineNumber) {
	// Get total number of lines
	int totalLines = SendMessage(hEdit, EM_GETLINECOUNT, 0, 0);
	
	// Validate line number (1-based)
	if (lineNumber < 1 || lineNumber > totalLines) {
		return FALSE;
	}
	
	// Convert to 0-based line index
	int lineIndex = lineNumber - 1;
	
	// Get character index of the beginning of the line
	int charIndex = SendMessage(hEdit, EM_LINEINDEX, lineIndex, 0);
	if (charIndex == -1) {
		return FALSE;
	}
	
	// Set cursor position to the beginning of the line
	SendMessage(hEdit, EM_SETSEL, charIndex, charIndex);
	
	// Scroll to make sure the line is visible
	SendMessage(hEdit, EM_SCROLLCARET, 0, 0);
	
	// Set focus to the edit control
	SetFocus(hEdit);
	SetForegroundWindow(GetParent(hEdit));
	
	return TRUE;
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

// Go To Dialog Procedure
INT_PTR CALLBACK GoToDialog(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam) {
	UNREFERENCED_PARAMETER(lParam);
	
	switch (message) {
	case WM_INITDIALOG:
		// Set focus to the line number text box
		SetFocus(GetDlgItem(hDlg, IDC_GOTO_LINE_NUMBER));
		return (INT_PTR)FALSE; // Return FALSE since we set focus manually

	case WM_COMMAND:
		switch (LOWORD(wParam)) {
		case IDC_GOTO_GO:
		case IDOK:
			{
				// Get the line number from the edit control
				TCHAR szLineNumber[32];
				GetDlgItemText(hDlg, IDC_GOTO_LINE_NUMBER, szLineNumber, 31);
				
				if (_tcslen(szLineNumber) == 0) {
					MessageBox(hDlg, _T("Please enter a line number."), _T("Go To Line"), MB_OK | MB_ICONINFORMATION);
					return (INT_PTR)TRUE;
				}

				// Convert to integer
				int lineNumber = _ttoi(szLineNumber);
				if (lineNumber <= 0) {
					MessageBox(hDlg, _T("Please enter a valid line number."), _T("Go To Line"), MB_OK | MB_ICONINFORMATION);
					return (INT_PTR)TRUE;
				}

				// Go to the specified line
				if (GoToLine(g_hEditControl, lineNumber)) {
					// Close dialog on success
					g_hGoToDialog = NULL;
					EndDialog(hDlg, LOWORD(wParam));
				} else {
					MessageBox(hDlg, _T("Line number is out of range."), _T("Go To Line"), MB_OK | MB_ICONINFORMATION);
				}
			}
			return (INT_PTR)TRUE;

		case IDC_GOTO_CLOSE:
		case IDCANCEL:
			g_hGoToDialog = NULL;
			EndDialog(hDlg, LOWORD(wParam));
			return (INT_PTR)TRUE;
		}
		break;

	case WM_CLOSE:
		g_hGoToDialog = NULL;
		EndDialog(hDlg, 0);
		return (INT_PTR)TRUE;
	}
	
	return (INT_PTR)FALSE;
}

// Show Go To Dialog
VOID ShowGoToDialog(HWND hWnd, HWND hEdit) {
	g_hEditControl = hEdit;
	
	// If dialog is already open, just bring it to front
	if (g_hGoToDialog && IsWindow(g_hGoToDialog)) {
		SetForegroundWindow(g_hGoToDialog);
		return;
	}

	// Create and show the dialog as a modal dialog
	g_hGoToDialog = CreateDialog(hInst, MAKEINTRESOURCE(IDD_GOTO_DIALOG), hWnd, GoToDialog);
	if (g_hGoToDialog) {
		ShowWindow(g_hGoToDialog, SW_SHOW);
		SetFocus(GetDlgItem(g_hGoToDialog, IDC_GOTO_LINE_NUMBER));
	}
}

// Initialize default font settings
VOID InitializeDefaultFont() {
	if (!g_bFontInitialized) {
		// Load font settings from registry (will set defaults if registry is empty)
		LoadFontFromRegistry(&g_currentLogFont);
		g_bFontInitialized = TRUE;
	}
}

// Apply font to edit control
VOID ApplyFontToEdit(HWND hEdit, LOGFONT* pLogFont) {
	if (!hEdit || !pLogFont) return;
	
	// Create font from LOGFONT
	HFONT hNewFont = CreateFontIndirect(pLogFont);
	if (hNewFont) {
		// Delete old font if it exists
		HFONT hOldFont = (HFONT)SendMessage(hEdit, WM_GETFONT, 0, 0);
		
		// Set new font
		SendMessage(hEdit, WM_SETFONT, (WPARAM)hNewFont, TRUE);
		
		// Delete old font (but not if it's the system font)
		if (hOldFont && hOldFont != g_hEditFont) {
			DeleteObject(hOldFont);
		}
		
		// Update global font handle
		if (g_hEditFont && g_hEditFont != hOldFont) {
			DeleteObject(g_hEditFont);
		}
		g_hEditFont = hNewFont;
		
		// Force edit control to repaint
		InvalidateRect(hEdit, NULL, TRUE);
		UpdateWindow(hEdit);
	}
}

// Show Font Dialog
VOID ShowFontDialog(HWND hWnd, HWND hEdit) {
	// Initialize default font if not done yet
	InitializeDefaultFont();
	
	CHOOSEFONT cf;
	ZeroMemory(&cf, sizeof(CHOOSEFONT));
	
	cf.lStructSize = sizeof(CHOOSEFONT);
	cf.hwndOwner = hWnd;
	cf.lpLogFont = &g_currentLogFont;
	cf.Flags = CF_SCREENFONTS | CF_EFFECTS | CF_INITTOLOGFONTSTRUCT;
	cf.nFontType = SCREEN_FONTTYPE;
	
	// Show the font dialog
	if (ChooseFont(&cf)) {
		// User selected a font, apply it to the edit control
		ApplyFontToEdit(hEdit, &g_currentLogFont);
		
		// Save the new font settings to registry
		SaveFontToRegistry(&g_currentLogFont);
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

// Text selection related function implementations
VOID SelectAllText(HWND hEdit) {
	if (!hEdit) return;
	
	// Select all text (0 to -1 means select all)
	SendMessage(hEdit, EM_SETSEL, 0, -1);
	
	// Ensure the edit control has focus
	SetFocus(hEdit);
}

VOID TestSelectAllFunction(HWND hEdit) {
	if (!hEdit) return;
	
	// Set some test text
	SetWindowText(hEdit, _T("This is test text\r\nSecond line\r\nThird line\r\nUsed to test the select all function"));
	
	// Delay a bit, then execute select all
	Sleep(500);
	SelectAllText(hEdit);
}

// Time/Date insert function implementation
VOID InsertDateTime(HWND hEdit) {
	if (!hEdit) return;
	
	// Get the current system time
	SYSTEMTIME st;
	GetLocalTime(&st);
	
	// Format the date and time string (using standard format: year/month/day hour:minute)
	TCHAR szDateTime[128];
	_stprintf_s(szDateTime, 128, _T("%04d/%02d/%02d %02d:%02d"), 
		st.wYear, st.wMonth, st.wDay, st.wHour, st.wMinute);
	
	// Insert date and time at the current cursor position
	SendMessage(hEdit, EM_REPLACESEL, TRUE, (LPARAM)szDateTime);
	
	// Ensure the edit control has focus
	SetFocus(hEdit);
}

// Initialize Word Wrap settings
VOID InitializeWordWrap() {
	LoadWordWrapFromRegistry(&g_bWordWrapEnabled);
}

// Toggle Word Wrap status
VOID ToggleWordWrap(HWND hWnd, HWND hEdit) {
	g_bWordWrapEnabled = !g_bWordWrapEnabled;
	SetWordWrap(hWnd, hEdit, g_bWordWrapEnabled);
	UpdateMenuWordWrap(hWnd);
}

// Get the current Word Wrap status
BOOL IsWordWrapEnabled() {
	return g_bWordWrapEnabled;
}

// Set Word Wrap status
VOID SetWordWrap(HWND hWnd, HWND hEdit, BOOL bEnable) {
	if (!hEdit) return;
	
	// Save the current text content
	int textLength = GetWindowTextLength(hEdit);
	LPTSTR pszText = NULL;
	
	if (textLength > 0) {
		pszText = (LPTSTR)LocalAlloc(LMEM_ZEROINIT, (textLength + 1) * sizeof(TCHAR));
		if (pszText) {
			GetWindowText(hEdit, pszText, textLength + 1);
		}
	}
	
	// Save the current selection position
	DWORD dwStart, dwEnd;
	SendMessage(hEdit, EM_GETSEL, (WPARAM)&dwStart, (LPARAM)&dwEnd);
	
	// Save the current first visible line number
	int nFirstVisibleLine = SendMessage(hEdit, EM_GETFIRSTVISIBLELINE, 0, 0);
	
	// Get the current window position and size
	RECT rcEdit;
	GetWindowRect(hEdit, &rcEdit);
	ScreenToClient(hWnd, (LPPOINT)&rcEdit);
	ScreenToClient(hWnd, (LPPOINT)&rcEdit + 1);
	
	// Destroy the current edit control
	DestroyWindow(hEdit);
	
	// Create a new edit control with style based on Word Wrap status
	DWORD dwStyle = WS_CHILD | WS_VISIBLE | WS_VSCROLL | ES_MULTILINE | ES_AUTOVSCROLL;
	
	if (!bEnable) {
		// If Word Wrap is not enabled, add horizontal scroll bar
		dwStyle |= WS_HSCROLL | ES_AUTOHSCROLL;
	}
	
	// Create the new edit control
	HWND hNewEdit = CreateWindowEx(
		WS_EX_CLIENTEDGE,
		_T("EDIT"),
		_T(""),
		dwStyle,
		rcEdit.left, rcEdit.top,
		rcEdit.right - rcEdit.left, rcEdit.bottom - rcEdit.top,
		hWnd,
		(HMENU)ID_EDIT,
		GetModuleHandle(NULL),
		NULL
	);
	
	if (hNewEdit) {
		// Update global handle
		extern HWND hwndEdit;
		hwndEdit = hNewEdit;
		
		// Apply font settings
		if (g_bFontInitialized) {
			ApplyFontToEdit(hNewEdit, &g_currentLogFont);
		} else {
			InitializeDefaultFont();
			ApplyFontToEdit(hNewEdit, &g_currentLogFont);
		}
		
		// Apply modern styling
		ApplyModernEditStyling(hNewEdit);
		
		// Restore text content
		if (pszText) {
			SetWindowText(hNewEdit, pszText);
			LocalFree(pszText);
		}
		
		// Restore selection position
		SendMessage(hNewEdit, EM_SETSEL, dwStart, dwEnd);
		
		// Restore scroll position to the first visible line
		if (nFirstVisibleLine > 0) {
			SendMessage(hNewEdit, EM_LINESCROLL, 0, nFirstVisibleLine);
		}
		
		// Set focus
		SetFocus(hNewEdit);
		
		// Update status bar information
		UpdateCursorPosition(hNewEdit);
		UpdateStatusBarWordWrap(bEnable);
	}
	
	g_bWordWrapEnabled = bEnable;
	
	// Save settings to the registry
	SaveWordWrapToRegistry(g_bWordWrapEnabled);
}

// Update the Word Wrap status in the menu
VOID UpdateMenuWordWrap(HWND hWnd) {
	HMENU hMenu = GetMenu(hWnd);
	if (hMenu) {
		HMENU hFormatMenu = GetSubMenu(hMenu, 2); // Format menu is the 3rd one (index 2)
		if (hFormatMenu) {
			UINT uFlags = MF_BYCOMMAND;
			if (g_bWordWrapEnabled) {
				uFlags |= MF_CHECKED;
			} else {
				uFlags |= MF_UNCHECKED;
			}
			CheckMenuItem(hFormatMenu, IDM_FORMAT_AUTOWRAP, uFlags);
		}
	}
}

// Update cursor position information to the status bar
VOID UpdateCursorPosition(HWND hEdit) {
	if (!hEdit || !IsWindow(hEdit)) {
		return;
	}
	
	// Get the current cursor position
	DWORD dwStart, dwEnd;
	SendMessage(hEdit, EM_GETSEL, (WPARAM)&dwStart, (LPARAM)&dwEnd);
	
	// Get the current line number (starting from 0)
	int lineNumber = SendMessage(hEdit, EM_LINEFROMCHAR, dwStart, 0);
	
	// Get the starting character position of the line
	int lineStart = SendMessage(hEdit, EM_LINEINDEX, lineNumber, 0);
	
	// Calculate the column number (starting from 0)
	int columnNumber = dwStart - lineStart;
	
	// Update the status bar (convert to display starting from 1)
	UpdateStatusBarPosition(lineNumber + 1, columnNumber + 1);
}
