#include "stdafx.h"
#include "Utils.h"
extern HWND hWnd;
VOID Error2Msgbox(DWORD dwError){
	HLOCAL hlocal = NULL; //Buffer that gets the error message string

	//Get the error code`s textual description
	BOOL fOk = FormatMessage(
		FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_ALLOCATE_BUFFER,
		NULL, dwError, MAKELANGID(LANG_CHINESE, SUBLANG_CHINESE_SIMPLIFIED),
		(LPTSTR)&hlocal, 0, NULL);
	if (hlocal != NULL){
		MessageBox(hWnd, (LPTSTR)hlocal, _T("Error Message is"), MB_OK);
		LocalFree(hlocal);
	}

}