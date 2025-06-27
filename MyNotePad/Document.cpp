#include "stdafx.h"
#include "Document.h"
#include "Utils.h"
#include "Encoding.h"

static LPTSTR hDocBuf;
static DWORD dwReaded;
static DWORD dwCurrentBufSize = 4096 * 8;
static EncodingType g_currentEncoding = ENCODING_UTF8; // Default encoding
void init_buff(){
	hDocBuf = (LPTSTR)LocalAlloc(LMEM_ZEROINIT, dwCurrentBufSize);

}
void free_buff(){
	if (hDocBuf){
		LocalFree(hDocBuf);
	}
}


VOID DO_OPEN_FILE(HWND hEdit, LPTSTR filename){
	// Use new encoding detection and conversion functionality
	LPWSTR content = NULL;
	DWORD contentLength = 0;
	EncodingType detectedEncoding = ENCODING_UNKNOWN;
	
	// Read the file and automatically detect encoding
	if (!ReadFileWithEncoding(filename, &content, &contentLength, &detectedEncoding)) {
		OutputDebugString(_T("Open File Failed - Encoding Detection"));
		Error2Msgbox(GetLastError());
		return;
	}
	
	// Save the currently detected encoding for use during saving
	g_currentEncoding = detectedEncoding;
	
	// Set text to the edit control
	SetWindowTextW(hEdit, content);
	
	// Clean up memory
	if (content) {
		LocalFree(content);
	}
	
	// Display encoding information in the status bar or title bar (optional)
	TCHAR title[512];
	LPCWSTR encodingName = GetEncodingName(detectedEncoding);
	_stprintf_s(title, 512, _T("MyNotePad - %s [%s]"), filename, encodingName);
	SetWindowText(GetParent(hEdit), title);
}

VOID DO_SAVE_FILE(HWND hEdit, LPTSTR filename){
	// Get the text length in the edit control
	int textLength = GetWindowTextLengthW(hEdit);
	if (textLength == 0) {
		// No content to save
		return;
	}

	// Allocate memory to store the text
	LPWSTR textBuffer = (LPWSTR)LocalAlloc(LMEM_ZEROINIT, (textLength + 1) * sizeof(WCHAR));
	if (!textBuffer) {
		OutputDebugString(_T("Failed to allocate memory for save buffer"));
		return;
	}

	// Get text from the edit control
	GetWindowTextW(hEdit, textBuffer, textLength + 1);

	// Save the file using the detected encoding
	if (!SaveFileWithEncoding(filename, textBuffer, textLength, g_currentEncoding)) {
		OutputDebugString(_T("Save File Failed"));
		Error2Msgbox(GetLastError());
	}

	// Clean up memory
	LocalFree(textBuffer);
}

// Encoding-related function implementations
EncodingType GetCurrentEncoding() {
	return g_currentEncoding;
}

VOID SetCurrentEncoding(EncodingType encoding) {
	g_currentEncoding = encoding;
}

LPCWSTR GetCurrentEncodingName() {
	return GetEncodingName(g_currentEncoding);
}
