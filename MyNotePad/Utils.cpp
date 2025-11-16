#include "stdafx.h"
#include "Utils.h"
extern HWND hWnd;
VOID Error2Msgbox(DWORD dwError){
	HLOCAL hlocal = NULL; //Buffer that gets the error message string

	//Get the error code's textual description
	if (FormatMessage(
		FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_ALLOCATE_BUFFER,
		NULL, dwError, MAKELANGID(LANG_ENGLISH, SUBLANG_ENGLISH_US),
		(LPTSTR)&hlocal, 0, NULL) && hlocal != NULL){
		MessageBox(hWnd, (LPTSTR)hlocal, _T("Error Message is"), MB_OK);
		LocalFree(hlocal);
	}

}