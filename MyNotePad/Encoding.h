#pragma once
#include <windows.h>

// 编码类型枚举
typedef enum {
    ENCODING_ANSI = 0,
    ENCODING_UTF8,
    ENCODING_UTF8_BOM,
    ENCODING_UTF16_LE,
    ENCODING_UTF16_BE,
    ENCODING_GBK,
    ENCODING_UNKNOWN
} EncodingType;

// 编码检测和转换函数
EncodingType DetectFileEncoding(const BYTE* buffer, DWORD size);
BOOL ConvertToUnicode(const BYTE* input, DWORD inputSize, EncodingType encoding, LPWSTR* output, DWORD* outputSize);
BOOL ConvertFromUnicode(LPCWSTR input, DWORD inputLength, EncodingType encoding, LPBYTE* output, DWORD* outputSize);
BOOL ReadFileWithEncoding(LPCTSTR filename, LPWSTR* content, DWORD* contentLength, EncodingType* detectedEncoding);
BOOL SaveFileWithEncoding(LPCTSTR filename, LPCWSTR content, DWORD contentLength, EncodingType encoding);

// 辅助函数
LPCWSTR GetEncodingName(EncodingType encoding);
BOOL IsValidUTF8(const BYTE* data, DWORD size);

// 换行符处理函数
LPWSTR NormalizeLineEndings(LPCWSTR input, DWORD inputLength, DWORD* outputLength);
LPWSTR ConvertLineEndingsToWindows(LPCWSTR input, DWORD inputLength, DWORD* outputLength);
