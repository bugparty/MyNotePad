#include "stdafx.h"
#include "Document.h"
#include "Utils.h"
#include "Encoding.h"

static LPTSTR hDocBuf;
static DWORD dwReaded;
static DWORD dwCurrentBufSize = 4096 * 8;
static EncodingType g_currentEncoding = ENCODING_UTF8; // 默认编码
void init_buff(){
	hDocBuf = (LPTSTR)LocalAlloc(LMEM_ZEROINIT, dwCurrentBufSize);

}
void free_buff(){
	if (hDocBuf){
		LocalFree(hDocBuf);
	}
}


VOID DO_OPEN_FILE(HWND hEdit, LPTSTR filename){
	// 使用新的编码检测和转换功能
	LPWSTR content = NULL;
	DWORD contentLength = 0;
	EncodingType detectedEncoding = ENCODING_UNKNOWN;
	
	// 读取文件并自动检测编码
	if (!ReadFileWithEncoding(filename, &content, &contentLength, &detectedEncoding)) {
		OutputDebugString(_T("Open File Failed - Encoding Detection"));
		Error2Msgbox(GetLastError());
		return;
	}
	
	// 保存当前检测到的编码，用于保存时使用
	g_currentEncoding = detectedEncoding;
	
	// 设置文本到编辑控件
	SetWindowTextW(hEdit, content);
	
	// 清理内存
	if (content) {
		LocalFree(content);
	}
	
	// 在状态栏或标题栏显示编码信息（可选）
	TCHAR title[512];
	LPCWSTR encodingName = GetEncodingName(detectedEncoding);
	_stprintf_s(title, 512, _T("MyNotePad - %s [%s]"), filename, encodingName);
	SetWindowText(GetParent(hEdit), title);
}

VOID DO_SAVE_FILE(HWND hEdit, LPTSTR filename){
	// 获取编辑控件中的文本长度
	int textLength = GetWindowTextLengthW(hEdit);
	if (textLength == 0) {
		// 没有内容需要保存
		return;
	}

	// 分配内存存储文本
	LPWSTR textBuffer = (LPWSTR)LocalAlloc(LMEM_ZEROINIT, (textLength + 1) * sizeof(WCHAR));
	if (!textBuffer) {
		OutputDebugString(_T("Failed to allocate memory for save buffer"));
		return;
	}

	// 从编辑控件获取文本
	GetWindowTextW(hEdit, textBuffer, textLength + 1);

	// 使用检测到的编码保存文件
	if (!SaveFileWithEncoding(filename, textBuffer, textLength, g_currentEncoding)) {
		OutputDebugString(_T("Save File Failed"));
		Error2Msgbox(GetLastError());
	}

	// 清理内存
	LocalFree(textBuffer);
}

// 编码相关函数实现
EncodingType GetCurrentEncoding() {
	return g_currentEncoding;
}

VOID SetCurrentEncoding(EncodingType encoding) {
	g_currentEncoding = encoding;
}

LPCWSTR GetCurrentEncodingName() {
	return GetEncodingName(g_currentEncoding);
}
