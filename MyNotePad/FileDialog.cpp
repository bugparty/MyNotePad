#include "stdafx.h"
#include "FileDialog.h"
#include "resource.h"
#include "Utils.h"
#include "Document.h"

// External global variables
extern HINSTANCE hInst;

// Global variables for file state management
static TCHAR g_szCurrentFileName[MAX_PATH] = {0};
static BOOL g_bHasFileName = FALSE;

// File dialog operations
VOID OpenDialogFileOpen(HWND hWnd, HWND hEdit) {
	OPENFILENAME ofn;
	TCHAR szFile[MAX_PATH];

	// Initialize OPENFILENAME
	ZeroMemory(&ofn, sizeof(ofn));
	ofn.lStructSize = sizeof(ofn);
	ofn.hwndOwner = hWnd;
	ofn.lpstrFile = szFile;
	// Set lpstrfile[0] to \0 so that GetOpenFileName does not use the contents of szFile to initialize itself.
	ofn.lpstrFile[0] = _T('\0');
	ofn.nMaxFile = sizeof(szFile);
	ofn.nFilterIndex = 1;
	ofn.lpstrFilter = _T("All\0*.*\0Text\0*.TXT\0");
	ofn.lpstrFileTitle = NULL;
	ofn.lpstrInitialDir = NULL;
	ofn.Flags = OFN_PATHMUSTEXIST | OFN_FILEMUSTEXIST;

	// Display the Open dialog box
	if (GetOpenFileName(&ofn) == TRUE) {
		DO_OPEN_FILE(hEdit, ofn.lpstrFile);
		// Store the opened file name for future save operations
		_tcscpy_s(g_szCurrentFileName, MAX_PATH, ofn.lpstrFile);
		g_bHasFileName = TRUE;
	}
	else {
		DWORD dwError = GetLastError();
		if (dwError == 0) return;
		Error2Msgbox(dwError);
	}
}

VOID OpenDialogFileSaveAs(HWND hWnd, HWND hEdit) {
	OPENFILENAME ofn;
	TCHAR szFile[MAX_PATH];

	// Initialize OPENFILENAME
	ZeroMemory(&ofn, sizeof(ofn));
	ofn.lStructSize = sizeof(ofn);
	ofn.hwndOwner = hWnd;
	ofn.lpstrFile = szFile;
	// Set lpstrfile[0] to \0 so that GetOpenFileName does not use the contents of szFile to initialize itself.
	ofn.lpstrFile[0] = '\0';
	ofn.nMaxFile = sizeof(szFile);
	ofn.nFilterIndex = 1;
	ofn.lpstrFilter = _T("All\0*.*\0Text\0*.TXT\0");
	ofn.lpstrFileTitle = NULL;
	ofn.lpstrInitialDir = NULL;
	ofn.Flags = OFN_PATHMUSTEXIST | OFN_OVERWRITEPROMPT | OFN_HIDEREADONLY;

	// Display the Save dialog box
	if (GetSaveFileName(&ofn) == TRUE) {
		DO_SAVE_FILE(hEdit, ofn.lpstrFile);
		// Store the saved file name for future save operations
		_tcscpy_s(g_szCurrentFileName, MAX_PATH, ofn.lpstrFile);
		g_bHasFileName = TRUE;
	}
	else {
		DWORD dwError = GetLastError();
		if (dwError == 0) return;
		Error2Msgbox(dwError);
	}
}

VOID OpenDialogFileSave(HWND hWnd, HWND hEdit) {
	// If we already have a file name, save directly to that file
	if (g_bHasFileName && g_szCurrentFileName[0] != '\0') {
		DO_SAVE_FILE(hEdit, g_szCurrentFileName);
	}
	else {
		// No file name yet, open Save As dialog
		OpenDialogFileSaveAs(hWnd, hEdit);
	}
}

// File state management functions
BOOL HasCurrentFileName() {
	return g_bHasFileName && g_szCurrentFileName[0] != '\0';
}

LPCTSTR GetCurrentFileName() {
	return g_szCurrentFileName;
}

VOID SetCurrentFileName(LPCTSTR filename) {
	if (filename && filename[0] != '\0') {
		_tcscpy_s(g_szCurrentFileName, MAX_PATH, filename);
		g_bHasFileName = TRUE;
	}
	else {
		ClearCurrentFileName();
	}
}

VOID ClearCurrentFileName() {
	g_szCurrentFileName[0] = '\0';
	g_bHasFileName = FALSE;
}

// Command line file operations
BOOL OpenFileFromCommandLine(HWND hEdit, LPCTSTR filename) {
	if (!filename || filename[0] == '\0') {
		return FALSE;
	}
	
	// Check if file exists
	DWORD dwAttrib = GetFileAttributes(filename);
	if (dwAttrib == INVALID_FILE_ATTRIBUTES || (dwAttrib & FILE_ATTRIBUTE_DIRECTORY)) {
		// File doesn't exist or is a directory
		TCHAR szMessage[MAX_PATH + 100];
		_stprintf_s(szMessage, MAX_PATH + 100, _T("Cannot open file '%s'.\nThe file does not exist or is not accessible."), filename);
		MessageBox(GetParent(hEdit), szMessage, _T("File Error"), MB_OK | MB_ICONERROR);
		return FALSE;
	}
	
	// Try to open the file
	DO_OPEN_FILE(hEdit, (LPTSTR)filename);
	
	// Store the opened file name for future save operations
	_tcscpy_s(g_szCurrentFileName, MAX_PATH, filename);
	g_bHasFileName = TRUE;
	
	return TRUE;
}
