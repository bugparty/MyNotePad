#include "stdafx.h"
#include "Dialog.h"
#include "resource.h"
#include "Utils.h"
#include "Document.h"
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
		(HINSTANCE)GetWindowLong(hWnd, GWL_HINSTANCE),
		NULL);                /* pointer not needed     */

}
VOID OpenDialogFileOpen(HWND hWnd, HWND hEdit){
	OPENFILENAME ofn;
	TCHAR szFile[MAX_PATH];

	HANDLE hf;

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
	}
	else{
		DWORD dwError = GetLastError();
		Error2Msgbox(dwError);
	}

}
VOID OpenDialogFileSaveAs(HWND hWnd){
	OPENFILENAME ofn;
	TCHAR szFile[MAX_PATH];

	HANDLE hf;

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

	//Display the Open dialog box
	if (GetSaveFileName(&ofn) == TRUE){
		hf = CreateFile(ofn.lpstrFile,
			GENERIC_READ,
			0,
			(LPSECURITY_ATTRIBUTES)NULL,
			OPEN_ALWAYS,
			FILE_ATTRIBUTE_NORMAL,
			(HANDLE)NULL);
	}
	else{
		DWORD dwError = GetLastError();
		Error2Msgbox(dwError);
	}

}