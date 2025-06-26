#pragma once

// External declarations for dialog windows
extern HWND g_hFindDialog;
extern HWND g_hReplaceDialog;

INT_PTR CALLBACK About(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam);
INT_PTR CALLBACK FindDialog(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam);
INT_PTR CALLBACK ReplaceDialog(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam);
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
VOID ShowReplaceDialog(HWND hWnd, HWND hEdit);
VOID TestSelection(HWND hEdit);

// 文本选择相关函数
VOID SelectAllText(HWND hEdit);
VOID TestSelectAllFunction(HWND hEdit);

// 打印相关函数
VOID ShowPrintDialog(HWND hWnd, HWND hEdit);
VOID ShowPageSetupDialog(HWND hWnd);
BOOL PrintTextDocument(HWND hWnd, HWND hEdit, HDC hPrinterDC);
VOID CalculateTextMetrics(HDC hDC, TEXTMETRIC* tm, int* linesPerPage, int* printableWidth);
int CalculateLineLength(HDC hDC, LPCTSTR text, int maxWidth);
BOOL PrintTextPage(HDC hDC, HWND hEdit, int startLine, int endLine, int pageNumber);