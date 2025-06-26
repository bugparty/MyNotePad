#pragma once

// File dialog operations
VOID OpenDialogFileOpen(HWND hWnd, HWND hEdit);
VOID OpenDialogFileSaveAs(HWND hWnd, HWND hEdit);
VOID OpenDialogFileSave(HWND hWnd, HWND hEdit);

// Command line file operations
BOOL OpenFileFromCommandLine(HWND hEdit, LPCTSTR filename);

// File state management
BOOL HasCurrentFileName();
LPCTSTR GetCurrentFileName();
VOID SetCurrentFileName(LPCTSTR filename);
VOID ClearCurrentFileName();
