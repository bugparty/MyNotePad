#pragma once

#include <windows.h>
#include <tchar.h>
#include <winver.h>

// 使用系统定义的 RTL_OSVERSIONINFOW
// 现代 Windows SDK 已经包含此定义

// About Dialog 相关函数声明
INT_PTR CALLBACK About(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam);
VOID GetWindowsVersionString(LPTSTR lpszVersionString, DWORD dwBufferSize);
VOID GetProductVersionInfo(LPTSTR lpszProductInfo, DWORD dwSize);
VOID UpdateAboutDialogInfo(HWND hDlg);
VOID CenterDialog(HWND hDlg);
VOID GetCPUInfo(LPTSTR lpszCPUInfo, DWORD dwBufferSize);
VOID GetMemoryInfo(LPTSTR lpszMemoryInfo, DWORD dwBufferSize);
