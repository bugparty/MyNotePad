#pragma once

// External declarations for dialog windows
extern HWND g_hFindDialog;
extern HWND g_hReplaceDialog;

INT_PTR CALLBACK About(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam);
INT_PTR CALLBACK FindDialog(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam);
INT_PTR CALLBACK ReplaceDialog(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam);
INT_PTR CALLBACK GoToDialog(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam);
VOID CreateEditControl(HWND &hwndEdit, HWND hWnd);
VOID ApplyModernEditStyling(HWND hwndEdit);
VOID CleanupEditResources();
VOID OpenDialogFileOpen(HWND hWnd, HWND hEdit);
VOID OpenDialogFileSaveAs(HWND hWnd, HWND hEdit);
VOID OpenDialogFileSave(HWND hWnd, HWND hEdit);
VOID ShowFindDialog(HWND hWnd, HWND hEdit);
VOID FindNext(HWND hEdit);
VOID ShowGoToDialog(HWND hWnd, HWND hEdit);
BOOL GoToLine(HWND hEdit, int lineNumber);
VOID ShowFontDialog(HWND hWnd, HWND hEdit);
VOID InitializeDefaultFont();
VOID ApplyFontToEdit(HWND hEdit, LOGFONT* pLogFont);
VOID ShowReplaceDialog(HWND hWnd, HWND hEdit);
VOID TestSelection(HWND hEdit);

// 文本选择相关函数
VOID SelectAllText(HWND hEdit);
VOID TestSelectAllFunction(HWND hEdit);