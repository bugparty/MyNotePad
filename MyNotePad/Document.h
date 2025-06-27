#pragma once
#include "Encoding.h"

VOID DO_OPEN(TCHAR* filename);
VOID DO_SAVE(TCHAR* filename);
VOID DO_OPEN_FILE(HWND hEdit, LPTSTR filename);
VOID DO_SAVE_FILE(HWND hEdit, LPTSTR filename);

// Encoding related functions
EncodingType GetCurrentEncoding();
VOID SetCurrentEncoding(EncodingType encoding);
LPCWSTR GetCurrentEncodingName();