// NotePadWTL.cpp : Defines the entry point for the application.
//

#include "pch.h"
#include "framework.h"
#include "NotePadWTL.h"
#include "MainWindow.h"

#define MAX_LOADSTRING 100
CAppModule  _Module;
// Global Variables:
HINSTANCE hInst;                                // current instance
WCHAR szTitle[MAX_LOADSTRING];                  // The title bar text
WCHAR szWindowClass[MAX_LOADSTRING];            // the main window class name

// Forward declarations of functions included in this code module:
ATOM                MyRegisterClass(HINSTANCE hInstance);
BOOL                InitInstance(HINSTANCE, int);
LRESULT CALLBACK    WndProc(HWND, UINT, WPARAM, LPARAM);
INT_PTR CALLBACK    About(HWND, UINT, WPARAM, LPARAM);

int APIENTRY wWinMain(_In_ HINSTANCE hInstance,
    _In_opt_ HINSTANCE hPrevInstance,
    _In_ LPWSTR    lpCmdLine,
    _In_ int       nCmdShow)
{
    UNREFERENCED_PARAMETER(hPrevInstance);
    UNREFERENCED_PARAMETER(lpCmdLine);
    
    HRESULT hRes = _Module.Init(nullptr, hInstance);
    ATLASSERT(SUCCEEDED(hRes));
    CMessageLoop theLoop;
    _Module.AddMessageLoop(&theLoop);
    AtlInitCommonControls(ICC_BAR_CLASSES);
    // Initialize global strings
    LoadStringW(hInstance, IDS_APP_TITLE, szTitle, MAX_LOADSTRING);
	CMainWindow wndMain;
	if (wndMain.Create(nullptr, CWindow::rcDefault, szTitle) == nullptr)
    {
        return -1;
    }
	wndMain.ShowWindow(nCmdShow);
	wndMain.UpdateWindow();
    HACCEL hAccelTable = LoadAccelerators(hInstance, MAKEINTRESOURCE(IDC_NOTEPADWTL));
	// Run the message loop
    int nRet = theLoop.Run();
    _Module.RemoveMessageLoop();

	// Cleanup
    _Module.Term();
	return nRet;
}


