#include "stdafx.h"
#include "StatusBar.h"
#include "Registry.h"
#include "Resource.h"
#include "Dialog.h"
#include "StatusBar.h"
#include "Registry.h"
#include "Resource.h"

// 状态栏相关全局变量
static HWND g_hStatusBar = NULL;
static BOOL g_bStatusBarVisible = TRUE;
static BOOL g_bStatusBarInitialized = FALSE;

// 状态栏各部分的宽度 (像素)
static int g_statusPartWidths[STATUS_PART_COUNT];

// 初始化状态栏设置
VOID InitializeStatusBar() {
	if (g_bStatusBarInitialized) return;
	
	// 从注册表加载状态栏可见性设置
	LoadStatusBarFromRegistry(&g_bStatusBarVisible);
	
	g_bStatusBarInitialized = TRUE;
}

// 创建状态栏
VOID CreateStatusBar(HWND hParent) {
	if (g_hStatusBar) {
		DestroyWindow(g_hStatusBar);
		g_hStatusBar = NULL;
	}
	
	// 只有在可见状态下才创建状态栏
	if (!g_bStatusBarVisible) {
		return;
	}
	
	// 创建状态栏控件
	g_hStatusBar = CreateWindowEx(
		0,                          // 扩展样式
		STATUSCLASSNAME,            // 状态栏类名
		NULL,                       // 窗口文本
		WS_CHILD | WS_VISIBLE | SBARS_SIZEGRIP, // 窗口样式，包含大小调整握柄
		0, 0, 0, 0,                // 位置和大小（会自动调整）
		hParent,                    // 父窗口
		NULL,                       // 菜单句柄
		GetModuleHandle(NULL),      // 实例句柄
		NULL                        // 创建参数
	);
	
	if (g_hStatusBar) {
		// 更新状态栏布局
		UpdateStatusBarLayout(hParent);
		
		// 设置初始文本
		UpdateStatusBarGeneral(_T("Ready"));
		UpdateStatusBarPosition(1, 1);
		UpdateStatusBarEncoding(_T("UTF-8"));
		UpdateStatusBarWordWrap(IsWordWrapEnabled()); // 从Dialog模块获取实际的Word Wrap状态
	}
}

// 销毁状态栏
VOID DestroyStatusBar() {
	if (g_hStatusBar) {
		DestroyWindow(g_hStatusBar);
		g_hStatusBar = NULL;
	}
}

// 更新状态栏布局
VOID UpdateStatusBarLayout(HWND hParent) {
	if (!g_hStatusBar || !IsWindow(g_hStatusBar)) {
		return;
	}
	
	// 首先让状态栏重新定位自己
	SendMessage(g_hStatusBar, WM_SIZE, 0, 0);
	
	// 获取父窗口客户区大小
	RECT rcClient;
	GetClientRect(hParent, &rcClient);
	int totalWidth = rcClient.right - rcClient.left;
	
	// 计算各部分的宽度
	g_statusPartWidths[STATUS_PART_GENERAL] = totalWidth * 50 / 100;    // 50% - 常规信息
	g_statusPartWidths[STATUS_PART_POSITION] = totalWidth * 20 / 100;   // 20% - 位置信息
	g_statusPartWidths[STATUS_PART_ENCODING] = totalWidth * 15 / 100;   // 15% - 编码信息
	g_statusPartWidths[STATUS_PART_WORDWRAP] = totalWidth * 15 / 100;   // 15% - 自动换行
	
	// 计算累积宽度用于设置分割点
	int partRightEdges[STATUS_PART_COUNT];
	partRightEdges[0] = g_statusPartWidths[0];
	for (int i = 1; i < STATUS_PART_COUNT; i++) {
		partRightEdges[i] = partRightEdges[i-1] + g_statusPartWidths[i];
	}
	
	// 最后一部分扩展到窗口右边缘
	partRightEdges[STATUS_PART_COUNT - 1] = -1;
	
	// 设置状态栏的分割
	SendMessage(g_hStatusBar, SB_SETPARTS, STATUS_PART_COUNT, (LPARAM)partRightEdges);
}

// 切换状态栏可见性
VOID ToggleStatusBar(HWND hParent) {
	g_bStatusBarVisible = !g_bStatusBarVisible;
	
	if (g_bStatusBarVisible) {
		// 显示状态栏
		CreateStatusBar(hParent);
	} else {
		// 隐藏状态栏
		DestroyStatusBar();
	}
	
	// 保存设置到注册表
	SaveStatusBarToRegistry(g_bStatusBarVisible);
	
	// 更新菜单状态
	UpdateMenuStatusBar(hParent);
	
	// 重新调整编辑控件的大小
	RECT rcClient;
	GetClientRect(hParent, &rcClient);
	PostMessage(hParent, WM_SIZE, SIZE_RESTORED, MAKELPARAM(rcClient.right, rcClient.bottom));
}

// 获取状态栏可见性
BOOL IsStatusBarVisible() {
	return g_bStatusBarVisible;
}

// 更新菜单中的状态栏状态
VOID UpdateMenuStatusBar(HWND hWnd) {
	HMENU hMenu = GetMenu(hWnd);
	if (hMenu) {
		HMENU hViewMenu = GetSubMenu(hMenu, 3); // View 菜单是第4个（索引3）
		if (hViewMenu) {
			UINT uFlags = MF_BYCOMMAND;
			if (g_bStatusBarVisible) {
				uFlags |= MF_CHECKED;
			} else {
				uFlags |= MF_UNCHECKED;
			}
			CheckMenuItem(hViewMenu, IDM_VIEW_STATUSBAR, uFlags);
		}
	}
}

// 更新状态栏位置信息
VOID UpdateStatusBarPosition(int line, int column) {
	if (!g_hStatusBar || !IsWindow(g_hStatusBar)) {
		return;
	}
	
	TCHAR positionText[64];
	_stprintf_s(positionText, 64, _T("Ln %d, Col %d"), line, column);
	SendMessage(g_hStatusBar, SB_SETTEXT, STATUS_PART_POSITION, (LPARAM)positionText);
}

// 更新状态栏编码信息
VOID UpdateStatusBarEncoding(LPCTSTR encoding) {
	if (!g_hStatusBar || !IsWindow(g_hStatusBar) || !encoding) {
		return;
	}
	
	SendMessage(g_hStatusBar, SB_SETTEXT, STATUS_PART_ENCODING, (LPARAM)encoding);
}

// 更新状态栏自动换行状态
VOID UpdateStatusBarWordWrap(BOOL bWordWrap) {
	if (!g_hStatusBar || !IsWindow(g_hStatusBar)) {
		return;
	}
	
	LPCTSTR wrapText = bWordWrap ? _T("Word Wrap") : _T("No Wrap");
	SendMessage(g_hStatusBar, SB_SETTEXT, STATUS_PART_WORDWRAP, (LPARAM)wrapText);
}

// 更新状态栏常规信息
VOID UpdateStatusBarGeneral(LPCTSTR message) {
	if (!g_hStatusBar || !IsWindow(g_hStatusBar) || !message) {
		return;
	}
	
	SendMessage(g_hStatusBar, SB_SETTEXT, STATUS_PART_GENERAL, (LPARAM)message);
}

// 从注册表加载状态栏可见性设置
VOID LoadStatusBarFromRegistry(BOOL* pStatusBarVisible) {
	if (!pStatusBarVisible) return;
	
	// 默认显示状态栏
	*pStatusBarVisible = TRUE;
	
	DWORD dwVisible = 1;
	if (ReadRegistryDWORD(HKEY_CURRENT_USER, REGISTRY_KEY_PATH, _T("StatusBarVisible"), &dwVisible)) {
		*pStatusBarVisible = (dwVisible != 0) ? TRUE : FALSE;
	}
}

// 保存状态栏可见性设置到注册表
VOID SaveStatusBarToRegistry(BOOL bStatusBarVisible) {
	DWORD dwVisible = bStatusBarVisible ? 1 : 0;
	WriteRegistryDWORD(HKEY_CURRENT_USER, REGISTRY_KEY_PATH, _T("StatusBarVisible"), dwVisible);
}

// 获取状态栏句柄
HWND GetStatusBarHandle() {
	return g_hStatusBar;
}
