#include "stdafx.h"
#include "Encoding.h"
#include <stdio.h>

// 检测文件编码
EncodingType DetectFileEncoding(const BYTE* buffer, DWORD size) {
    if (!buffer || size == 0) {
        return ENCODING_UNKNOWN;
    }

    // 检测 BOM (Byte Order Mark)
    if (size >= 3) {
        // UTF-8 BOM: EF BB BF
        if (buffer[0] == 0xEF && buffer[1] == 0xBB && buffer[2] == 0xBF) {
            return ENCODING_UTF8_BOM;
        }
    }

    if (size >= 2) {
        // UTF-16 LE BOM: FF FE
        if (buffer[0] == 0xFF && buffer[1] == 0xFE) {
            return ENCODING_UTF16_LE;
        }
        // UTF-16 BE BOM: FE FF
        if (buffer[0] == 0xFE && buffer[1] == 0xFF) {
            return ENCODING_UTF16_BE;
        }
    }

    // 检测 UTF-8 (无BOM)
    if (IsValidUTF8(buffer, size)) {
        return ENCODING_UTF8;
    }

    // 检测中文编码 (简单检测)
    BOOL hasChineseBytes = FALSE;
    for (DWORD i = 0; i < size - 1; i++) {
        // GBK 范围: A1-FE A1-FE
        if (buffer[i] >= 0xA1 && buffer[i] <= 0xFE && 
            buffer[i + 1] >= 0xA1 && buffer[i + 1] <= 0xFE) {
            hasChineseBytes = TRUE;
            break;
        }
    }

    if (hasChineseBytes) {
        return ENCODING_GBK;
    }

    // 默认为 ANSI
    return ENCODING_ANSI;
}

// 检查是否为有效的UTF-8
BOOL IsValidUTF8(const BYTE* data, DWORD size) {
    for (DWORD i = 0; i < size; ) {
        BYTE c = data[i];
        
        if (c <= 0x7F) {
            // ASCII
            i++;
        } else if ((c & 0xE0) == 0xC0) {
            // 110xxxxx - 2字节字符
            if (i + 1 >= size) return FALSE;
            if ((data[i + 1] & 0xC0) != 0x80) return FALSE;
            i += 2;
        } else if ((c & 0xF0) == 0xE0) {
            // 1110xxxx - 3字节字符
            if (i + 2 >= size) return FALSE;
            if ((data[i + 1] & 0xC0) != 0x80) return FALSE;
            if ((data[i + 2] & 0xC0) != 0x80) return FALSE;
            i += 3;
        } else if ((c & 0xF8) == 0xF0) {
            // 11110xxx - 4字节字符
            if (i + 3 >= size) return FALSE;
            if ((data[i + 1] & 0xC0) != 0x80) return FALSE;
            if ((data[i + 2] & 0xC0) != 0x80) return FALSE;
            if ((data[i + 3] & 0xC0) != 0x80) return FALSE;
            i += 4;
        } else {
            return FALSE;
        }
    }
    return TRUE;
}

// 将各种编码转换为Unicode
BOOL ConvertToUnicode(const BYTE* input, DWORD inputSize, EncodingType encoding, LPWSTR* output, DWORD* outputSize) {
    if (!input || !output || !outputSize) return FALSE;

    *output = NULL;
    *outputSize = 0;

    const BYTE* actualInput = input;
    DWORD actualSize = inputSize;
    UINT codePage = CP_ACP;

    // 跳过BOM并设置代码页
    switch (encoding) {
    case ENCODING_UTF8_BOM:
        if (inputSize >= 3) {
            actualInput += 3;
            actualSize -= 3;
        }
        codePage = CP_UTF8;
        break;
    
    case ENCODING_UTF8:
        codePage = CP_UTF8;
        break;
    
    case ENCODING_UTF16_LE:
        if (inputSize >= 2) {
            actualInput += 2;
            actualSize -= 2;
        }
        // 直接复制，因为已经是UTF-16 LE
        *outputSize = actualSize / sizeof(WCHAR);
        *output = (LPWSTR)LocalAlloc(LMEM_ZEROINIT, actualSize + sizeof(WCHAR));
        if (*output) {
            memcpy(*output, actualInput, actualSize);
        }
        return (*output != NULL);
    
    case ENCODING_UTF16_BE:
        // 需要字节序转换
        if (inputSize >= 2) {
            actualInput += 2;
            actualSize -= 2;
        }
        *outputSize = actualSize / sizeof(WCHAR);
        *output = (LPWSTR)LocalAlloc(LMEM_ZEROINIT, actualSize + sizeof(WCHAR));
        if (*output) {
            LPWSTR dest = *output;
            LPCWSTR src = (LPCWSTR)actualInput;
            for (DWORD i = 0; i < *outputSize; i++) {
                dest[i] = ((src[i] & 0xFF) << 8) | ((src[i] & 0xFF00) >> 8);
            }
        }
        return (*output != NULL);
    
    case ENCODING_GBK:
        codePage = 936; // GBK code page
        break;
    
    case ENCODING_ANSI:
    default:
        codePage = CP_ACP;
        break;
    }

    // 计算需要的Unicode字符数
    int unicodeLength = MultiByteToWideChar(codePage, 0, (LPCSTR)actualInput, actualSize, NULL, 0);
    if (unicodeLength == 0) return FALSE;

    // 分配内存
    *output = (LPWSTR)LocalAlloc(LMEM_ZEROINIT, (unicodeLength + 1) * sizeof(WCHAR));
    if (!*output) return FALSE;

    // 转换
    int result = MultiByteToWideChar(codePage, 0, (LPCSTR)actualInput, actualSize, *output, unicodeLength);
    if (result == 0) {
        LocalFree(*output);
        *output = NULL;
        return FALSE;
    }

    *outputSize = result;
    return TRUE;
}

// 将Unicode转换为指定编码
BOOL ConvertFromUnicode(LPCWSTR input, DWORD inputLength, EncodingType encoding, LPBYTE* output, DWORD* outputSize) {
    if (!input || !output || !outputSize) return FALSE;

    *output = NULL;
    *outputSize = 0;

    UINT codePage = CP_ACP;
    BOOL needBOM = FALSE;
    DWORD bomSize = 0;
    BYTE bom[4] = {0};

    switch (encoding) {
    case ENCODING_UTF8_BOM:
        needBOM = TRUE;
        bomSize = 3;
        bom[0] = 0xEF; bom[1] = 0xBB; bom[2] = 0xBF;
        // fall through
    case ENCODING_UTF8:
        codePage = CP_UTF8;
        break;
    
    case ENCODING_UTF16_LE:
        needBOM = TRUE;
        bomSize = 2;
        bom[0] = 0xFF; bom[1] = 0xFE;
        
        *outputSize = inputLength * sizeof(WCHAR) + bomSize;
        *output = (LPBYTE)LocalAlloc(LMEM_ZEROINIT, *outputSize);
        if (*output) {
            if (needBOM) {
                memcpy(*output, bom, bomSize);
            }
            memcpy(*output + bomSize, input, inputLength * sizeof(WCHAR));
        }
        return (*output != NULL);
    
    case ENCODING_GBK:
        codePage = 936;
        break;
    
    case ENCODING_ANSI:
    default:
        codePage = CP_ACP;
        break;
    }

    // 计算需要的字节数
    int byteLength = WideCharToMultiByte(codePage, 0, input, inputLength, NULL, 0, NULL, NULL);
    if (byteLength == 0) return FALSE;

    // 分配内存 (包括BOM)
    *outputSize = byteLength + bomSize;
    *output = (LPBYTE)LocalAlloc(LMEM_ZEROINIT, *outputSize);
    if (!*output) return FALSE;

    // 添加BOM
    if (needBOM && bomSize > 0) {
        memcpy(*output, bom, bomSize);
    }

    // 转换
    int result = WideCharToMultiByte(codePage, 0, input, inputLength, 
                                   (LPSTR)(*output + bomSize), byteLength, NULL, NULL);
    if (result == 0) {
        LocalFree(*output);
        *output = NULL;
        return FALSE;
    }

    return TRUE;
}

// 读取文件并自动检测编码
BOOL ReadFileWithEncoding(LPCTSTR filename, LPWSTR* content, DWORD* contentLength, EncodingType* detectedEncoding) {
    if (!filename || !content || !contentLength) return FALSE;

    *content = NULL;
    *contentLength = 0;
    if (detectedEncoding) *detectedEncoding = ENCODING_UNKNOWN;

    // 打开文件
    HANDLE hFile = CreateFile(filename, GENERIC_READ, FILE_SHARE_READ, NULL, 
                             OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
    if (hFile == INVALID_HANDLE_VALUE) return FALSE;

    // 获取文件大小
    DWORD fileSize = GetFileSize(hFile, NULL);
    if (fileSize == INVALID_FILE_SIZE) {
        CloseHandle(hFile);
        return FALSE;
    }

    // 读取文件内容
    LPBYTE buffer = (LPBYTE)LocalAlloc(LMEM_ZEROINIT, fileSize);
    if (!buffer) {
        CloseHandle(hFile);
        return FALSE;
    }

    DWORD bytesRead;
    BOOL readResult = ReadFile(hFile, buffer, fileSize, &bytesRead, NULL);
    CloseHandle(hFile);

    if (!readResult || bytesRead != fileSize) {
        LocalFree(buffer);
        return FALSE;
    }

    // 检测编码
    EncodingType encoding = DetectFileEncoding(buffer, bytesRead);
    if (detectedEncoding) *detectedEncoding = encoding;

    // 转换为Unicode
    LPWSTR rawContent = NULL;
    DWORD rawContentLength = 0;
    BOOL convertResult = ConvertToUnicode(buffer, bytesRead, encoding, &rawContent, &rawContentLength);
    
    LocalFree(buffer);
    
    if (!convertResult || !rawContent) {
        return FALSE;
    }
    
    // 标准化换行符 - 将所有换行符转换为Windows格式(\r\n)
    DWORD normalizedLength = 0;
    LPWSTR normalizedContent = NormalizeLineEndings(rawContent, rawContentLength, &normalizedLength);
    
    // 清理原始内容
    LocalFree(rawContent);
    
    if (!normalizedContent) {
        return FALSE;
    }
    
    *content = normalizedContent;
    *contentLength = normalizedLength;
    
    return TRUE;
}

// 保存文件为指定编码
BOOL SaveFileWithEncoding(LPCTSTR filename, LPCWSTR content, DWORD contentLength, EncodingType encoding) {
    if (!filename || !content) return FALSE;

    // 转换编码
    LPBYTE output;
    DWORD outputSize;
    if (!ConvertFromUnicode(content, contentLength, encoding, &output, &outputSize)) {
        return FALSE;
    }

    // 保存文件
    HANDLE hFile = CreateFile(filename, GENERIC_WRITE, 0, NULL, 
                             CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
    if (hFile == INVALID_HANDLE_VALUE) {
        LocalFree(output);
        return FALSE;
    }

    DWORD bytesWritten;
    BOOL writeResult = WriteFile(hFile, output, outputSize, &bytesWritten, NULL);
    
    CloseHandle(hFile);
    LocalFree(output);
    
    return writeResult && (bytesWritten == outputSize);
}

// 获取编码名称
LPCWSTR GetEncodingName(EncodingType encoding) {
    switch (encoding) {
    case ENCODING_ANSI: return L"ANSI";
    case ENCODING_UTF8: return L"UTF-8";
    case ENCODING_UTF8_BOM: return L"UTF-8 with BOM";
    case ENCODING_UTF16_LE: return L"UTF-16 LE";
    case ENCODING_UTF16_BE: return L"UTF-16 BE";
    case ENCODING_GBK: return L"GBK";
    default: return L"Unknown";
    }
}

// 将各种换行符格式统一转换为Windows格式 (\r\n)
LPWSTR NormalizeLineEndings(LPCWSTR input, DWORD inputLength, DWORD* outputLength) {
    if (!input || !outputLength) return NULL;
    
    *outputLength = 0;
    
    // 首先计算需要的内存大小
    DWORD estimatedSize = 0;
    for (DWORD i = 0; i < inputLength; i++) {
        if (input[i] == L'\r') {
            if (i + 1 < inputLength && input[i + 1] == L'\n') {
                // Windows格式 \r\n - 保持不变
                estimatedSize += 2;
                i++; // 跳过 \n
            } else {
                // Mac格式 \r - 转换为 \r\n
                estimatedSize += 2;
            }
        } else if (input[i] == L'\n') {
            // Unix格式 \n - 转换为 \r\n
            estimatedSize += 2;
        } else {
            estimatedSize += 1;
        }
    }
    
    // 分配内存
    LPWSTR output = (LPWSTR)LocalAlloc(LMEM_ZEROINIT, (estimatedSize + 1) * sizeof(WCHAR));
    if (!output) return NULL;
    
    // 执行转换
    DWORD outPos = 0;
    for (DWORD i = 0; i < inputLength; i++) {
        if (input[i] == L'\r') {
            if (i + 1 < inputLength && input[i + 1] == L'\n') {
                // Windows格式 \r\n - 直接复制
                output[outPos++] = L'\r';
                output[outPos++] = L'\n';
                i++; // 跳过 \n
            } else {
                // Mac格式 \r - 转换为 \r\n
                output[outPos++] = L'\r';
                output[outPos++] = L'\n';
            }
        } else if (input[i] == L'\n') {
            // Unix格式 \n - 转换为 \r\n
            output[outPos++] = L'\r';
            output[outPos++] = L'\n';
        } else {
            // 普通字符
            output[outPos++] = input[i];
        }
    }
    
    *outputLength = outPos;
    return output;
}

// 简化版本，主要用于将Unix/Mac换行符转换为Windows格式
LPWSTR ConvertLineEndingsToWindows(LPCWSTR input, DWORD inputLength, DWORD* outputLength) {
    return NormalizeLineEndings(input, inputLength, outputLength);
}
