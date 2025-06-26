#pragma once

#include <windows.h>
#include <tchar.h>

// Registry constants for storing font settings
#define REGISTRY_KEY_PATH  _T("Software\\MyNotePad")
#define FONT_FACE_NAME     _T("FontFaceName")
#define FONT_HEIGHT        _T("FontHeight")
#define FONT_WEIGHT        _T("FontWeight")
#define FONT_ITALIC        _T("FontItalic")
#define FONT_UNDERLINE     _T("FontUnderline")
#define FONT_STRIKEOUT     _T("FontStrikeOut")
#define FONT_CHARSET       _T("FontCharSet")

// Word Wrap registry constants
#define WORDWRAP_ENABLED   _T("WordWrapEnabled")

// Font registry functions
VOID LoadFontFromRegistry(LOGFONT* pLogFont);
VOID SaveFontToRegistry(const LOGFONT* pLogFont);

// Word Wrap registry functions  
VOID LoadWordWrapFromRegistry(BOOL* pWordWrap);
VOID SaveWordWrapToRegistry(BOOL bWordWrap);

// General registry utility functions
BOOL CreateRegistryKey(HKEY hKey, LPCTSTR lpSubKey);
BOOL DeleteRegistryKey(HKEY hKey, LPCTSTR lpSubKey);
BOOL ReadRegistryString(HKEY hKey, LPCTSTR lpSubKey, LPCTSTR lpValueName, LPTSTR lpData, DWORD dwSize);
BOOL WriteRegistryString(HKEY hKey, LPCTSTR lpSubKey, LPCTSTR lpValueName, LPCTSTR lpData);
BOOL ReadRegistryDWORD(HKEY hKey, LPCTSTR lpSubKey, LPCTSTR lpValueName, DWORD* pValue);
BOOL WriteRegistryDWORD(HKEY hKey, LPCTSTR lpSubKey, LPCTSTR lpValueName, DWORD value);
