#pragma once

#include <windows.h>
#include <commctrl.h>
#include <tchar.h>

// 状态栏部分定义
#define STATUS_PART_GENERAL    0    // 常规信息
#define STATUS_PART_POSITION   1    // 光标位置 (行:列)
#define STATUS_PART_ENCODING   2    // 文件编码
#define STATUS_PART_WORDWRAP   3    // 自动换行状态
#define STATUS_PART_COUNT      4    // 总部分数

// 状态栏相关函数声明
VOID InitializeStatusBar();
VOID CreateStatusBar(HWND hParent);
VOID DestroyStatusBar();
VOID UpdateStatusBarLayout(HWND hParent);
VOID ToggleStatusBar(HWND hParent);
BOOL IsStatusBarVisible();
VOID UpdateMenuStatusBar(HWND hWnd);

// 状态栏信息更新函数
VOID UpdateStatusBarPosition(int line, int column);
VOID UpdateStatusBarEncoding(LPCTSTR encoding);
VOID UpdateStatusBarWordWrap(BOOL bWordWrap);
VOID UpdateStatusBarGeneral(LPCTSTR message);

// 注册表相关函数
VOID LoadStatusBarFromRegistry(BOOL* pStatusBarVisible);
VOID SaveStatusBarToRegistry(BOOL bStatusBarVisible);

// 获取状态栏句柄
HWND GetStatusBarHandle();
