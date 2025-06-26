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
// Creation of main EditControl
VOID CreateEditControl(HWND &hwndEdit, HWND hWnd){
	hwndEdit = CreateWindow(
		_T("EDIT"),     /* predefined class                  */
		NULL,       /* no window title                   */
		WS_CHILD | WS_VISIBLE | WS_HSCROLL | ES_MULTILINE |
		ES_LEFT | WS_VSCROLL 
		,
		0, 0, 300, 220, /* set size in WM_SIZE message       */
		hWnd,       /* parent window                     */
		(HMENU)ID_EDIT, /* edit control ID         */
		(HINSTANCE)GetWindowLongPtr(hWnd, GWLP_HINSTANCE),
		NULL);                /* pointer not needed     */

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

	// Debug output
	TCHAR debugMsg[512];
	_stprintf_s(debugMsg, 512, _T("Searching for: '%s', TextLength: %d"), findText, textLen);
	OutputDebugString(debugMsg);

	// Get all text from edit control
	LPTSTR buffer = (LPTSTR)LocalAlloc(LMEM_ZEROINIT, (textLen + 1) * sizeof(TCHAR));
	if (!buffer) return FALSE;

	GetWindowText(hEdit, buffer, textLen + 1);

	// Get current selection
	DWORD selStart, selEnd;
	SendMessage(hEdit, EM_GETSEL, (WPARAM)&selStart, (LPARAM)&selEnd);

	// Debug output
	_stprintf_s(debugMsg, 512, _T("Current selection: %d to %d"), selStart, selEnd);
	OutputDebugString(debugMsg);

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
		// Debug output
		TCHAR debugMsg[256];
		_stprintf_s(debugMsg, 256, _T("Found text at position: %d, length: %d"), foundPos, _tcslen(findText));
		OutputDebugString(debugMsg);
		
		// IMPORTANT: Set focus to edit control AND bring parent window to front
		SetFocus(hEdit);
		SetForegroundWindow(GetParent(hEdit));
		
		// Set selection using proper casting
		LRESULT result = SendMessage(hEdit, EM_SETSEL, (WPARAM)foundPos, (LPARAM)(foundPos + _tcslen(findText)));
		
		// Debug the result
		_stprintf_s(debugMsg, 256, _T("EM_SETSEL result: %d"), result);
		OutputDebugString(debugMsg);
		
		// Scroll to make sure the selection is visible
		SendMessage(hEdit, EM_SCROLLCARET, 0, 0);
		
		// Force a repaint to ensure the selection is visible
		InvalidateRect(hEdit, NULL, TRUE);
		UpdateWindow(hEdit);
		
		// Verify the selection was set
		DWORD newSelStart, newSelEnd;
		SendMessage(hEdit, EM_GETSEL, (WPARAM)&newSelStart, (LPARAM)&newSelEnd);
		_stprintf_s(debugMsg, 256, _T("New selection: %d to %d"), newSelStart, newSelEnd);
		OutputDebugString(debugMsg);
		
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

// Simple test function to verify selection works
VOID TestSelection(HWND hEdit) {
	// Test if basic selection works
	SetFocus(hEdit);
	SendMessage(hEdit, EM_SETSEL, 0, 5); // Select first 5 characters
	InvalidateRect(hEdit, NULL, TRUE);
	UpdateWindow(hEdit);
	OutputDebugString(_T("Test selection: first 5 characters should be selected"));
}