#pragma once

INT_PTR CALLBACK About(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam);
VOID CreateEditControl(HWND &hwndEdit, HWND hWnd);
VOID OpenDialogFileOpen(HWND hWnd);
VOID OpenDialogFileSaveAs(HWND hWnd);