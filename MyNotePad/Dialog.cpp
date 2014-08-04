#include "stdafx.h"
#include "Dialog.h"
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
// 主EditControl的创建
VOID CreateEditControl(HWND &hwndEdit, HWND hWnd){
	hwndEdit = CreateWindow(
		_T("EDIT"),     /* predefined class                  */
		NULL,       /* no window title                   */
		WS_CHILD | WS_VISIBLE | WS_HSCROLL | ES_MULTILINE |
		ES_LEFT | WS_VSCROLL,
		0, 0, 300, 220, /* set size in WM_SIZE message       */
		hWnd,       /* parent window                     */
		(HMENU)1, /* edit control ID         */
		(HINSTANCE)GetWindowLong(hWnd, GWL_HINSTANCE),
		NULL);                /* pointer not needed     */

}