#pragma once

#include <windows.h>
#include <tchar.h>
#include <winver.h>

// About Dialog related function declarations
INT_PTR CALLBACK About(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam);
VOID GetWindowsVersionString(LPTSTR lpszVersionString, DWORD dwBufferSize);
VOID GetProductVersionInfo(LPTSTR lpszProductInfo, DWORD dwSize);
VOID UpdateAboutDialogInfo(HWND hDlg);
VOID CenterDialog(HWND hDlg);
VOID GetCPUInfo(LPTSTR lpszCPUInfo, DWORD dwBufferSize);
VOID GetMemoryInfo(LPTSTR lpszMemoryInfo, DWORD dwBufferSize);
