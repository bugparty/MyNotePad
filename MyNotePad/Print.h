#pragma once

#include <windows.h>
#include <commdlg.h>
#include <tchar.h>

// 打印相关函数声明
VOID InitializePrintSettings(HWND hEdit);
VOID ShowPrintDialog(HWND hWnd, HWND hEdit);
VOID ShowPageSetupDialog(HWND hWnd);
BOOL PrintTextDocument(HWND hWnd, HWND hEdit, HDC hPrinterDC);
VOID CalculateTextMetrics(HDC hDC, TEXTMETRIC* tm, int* linesPerPage, int* printableWidth);
int CalculateLineLength(HDC hDC, LPCTSTR text, int maxWidth);
BOOL PrintTextPage(HDC hDC, HWND hEdit, int startLine, int endLine, int pageNumber);
BOOL DrawPrintPreview(HDC hPrnDC, HWND hEdit, int pageNo, HWND hPreviewWnd);

