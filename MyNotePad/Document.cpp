#include "stdafx.h"
#include "Document.h"
#include "Utils.h"
static LPTSTR hDocBuf;
static DWORD dwReaded;
static DWORD dwCurrentBufSize = 4096 * 8;
void init_buff(){
	hDocBuf = (LPTSTR)LocalAlloc(LMEM_ZEROINIT, dwCurrentBufSize);

}
void free_buff(){
	if (hDocBuf){
		LocalFree(hDocBuf);
	}
}


VOID DO_OPEN_FILE(HWND hEdit, LPTSTR filename){
	/*
	HANDLE WINAPI CreateFile(
	_In_      LPCTSTR lpFileName,
	_In_      DWORD dwDesiredAccess,
	_In_      DWORD dwShareMode,
	_In_opt_  LPSECURITY_ATTRIBUTES lpSecurityAttributes,
	_In_      DWORD dwCreationDisposition,
	_In_      DWORD dwFlagsAndAttributes,
	_In_opt_  HANDLE hTemplateFile
	);
	*/
	HANDLE hFile = CreateFile(filename,
		GENERIC_READ ,
		NULL,
		NULL,
		OPEN_EXISTING | CREATE_NEW,
		FILE_ATTRIBUTE_NORMAL,
		NULL);
	if (hFile == INVALID_HANDLE_VALUE){
		OutputDebugString(_T("Open File Failed"));
		Error2Msgbox(GetLastError());
		return;
	}
	init_buff();
	DWORD bytes_read;
	LPTSTR pointer = hDocBuf;
	do{
		ReadFile(hFile, (LPVOID)pointer, 4096, &bytes_read, NULL);
		pointer += bytes_read;

	} while (bytes_read == 4096);
	CloseHandle(hFile);
	//TODO : stack overflow problems
	dwReaded = pointer - hDocBuf + 1;
	int ret = Edit_SetText(hEdit, hDocBuf);
	
	assert(ret != 0);
	free_buff();

}
