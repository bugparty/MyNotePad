#include "stdafx.h"
#include "Registry.h"

// Load font settings from registry
VOID LoadFontFromRegistry(LOGFONT* pLogFont) {
	if (!pLogFont) return;
	
	HKEY hKey;
	DWORD dwType, dwSize;
	
	// Initialize with default values first
	ZeroMemory(pLogFont, sizeof(LOGFONT));
	pLogFont->lfHeight = -12;
	pLogFont->lfWeight = FW_NORMAL;
	pLogFont->lfCharSet = DEFAULT_CHARSET;
	pLogFont->lfOutPrecision = OUT_DEFAULT_PRECIS;
	pLogFont->lfClipPrecision = CLIP_DEFAULT_PRECIS;
	pLogFont->lfQuality = CLEARTYPE_QUALITY;
	pLogFont->lfPitchAndFamily = VARIABLE_PITCH | FF_DONTCARE;
	_tcscpy_s(pLogFont->lfFaceName, LF_FACESIZE, _T("Consolas"));
	
	// Try to open the registry key
	if (RegOpenKeyEx(HKEY_CURRENT_USER, REGISTRY_KEY_PATH, 0, KEY_READ, &hKey) == ERROR_SUCCESS) {
		// Load font face name
		dwSize = sizeof(pLogFont->lfFaceName);
		if (RegQueryValueEx(hKey, FONT_FACE_NAME, NULL, &dwType, (LPBYTE)pLogFont->lfFaceName, &dwSize) != ERROR_SUCCESS) {
			_tcscpy_s(pLogFont->lfFaceName, LF_FACESIZE, _T("Consolas")); // Default
		}
		
		// Load font height
		dwSize = sizeof(DWORD);
		DWORD dwHeight;
		if (RegQueryValueEx(hKey, FONT_HEIGHT, NULL, &dwType, (LPBYTE)&dwHeight, &dwSize) == ERROR_SUCCESS) {
			pLogFont->lfHeight = (LONG)dwHeight;
		}
		
		// Load font weight
		DWORD dwWeight;
		if (RegQueryValueEx(hKey, FONT_WEIGHT, NULL, &dwType, (LPBYTE)&dwWeight, &dwSize) == ERROR_SUCCESS) {
			pLogFont->lfWeight = (LONG)dwWeight;
		}
		
		// Load font italic
		DWORD dwItalic;
		if (RegQueryValueEx(hKey, FONT_ITALIC, NULL, &dwType, (LPBYTE)&dwItalic, &dwSize) == ERROR_SUCCESS) {
			pLogFont->lfItalic = (BYTE)dwItalic;
		}
		
		// Load font underline
		DWORD dwUnderline;
		if (RegQueryValueEx(hKey, FONT_UNDERLINE, NULL, &dwType, (LPBYTE)&dwUnderline, &dwSize) == ERROR_SUCCESS) {
			pLogFont->lfUnderline = (BYTE)dwUnderline;
		}
		
		// Load font strikeout
		DWORD dwStrikeOut;
		if (RegQueryValueEx(hKey, FONT_STRIKEOUT, NULL, &dwType, (LPBYTE)&dwStrikeOut, &dwSize) == ERROR_SUCCESS) {
			pLogFont->lfStrikeOut = (BYTE)dwStrikeOut;
		}
		
		// Load font charset
		DWORD dwCharSet;
		if (RegQueryValueEx(hKey, FONT_CHARSET, NULL, &dwType, (LPBYTE)&dwCharSet, &dwSize) == ERROR_SUCCESS) {
			pLogFont->lfCharSet = (BYTE)dwCharSet;
		}
		
		RegCloseKey(hKey);
	}
}

// Save font settings to registry
VOID SaveFontToRegistry(const LOGFONT* pLogFont) {
	if (!pLogFont) return;
	
	HKEY hKey;
	DWORD dwDisposition;
	
	// Create or open the registry key
	if (RegCreateKeyEx(HKEY_CURRENT_USER, REGISTRY_KEY_PATH, 0, NULL, REG_OPTION_NON_VOLATILE, KEY_WRITE, NULL, &hKey, &dwDisposition) == ERROR_SUCCESS) {
		// Save font face name
		RegSetValueEx(hKey, FONT_FACE_NAME, 0, REG_SZ, (LPBYTE)pLogFont->lfFaceName, (_tcslen(pLogFont->lfFaceName) + 1) * sizeof(TCHAR));
		
		// Save font height
		DWORD dwHeight = (DWORD)pLogFont->lfHeight;
		RegSetValueEx(hKey, FONT_HEIGHT, 0, REG_DWORD, (LPBYTE)&dwHeight, sizeof(DWORD));
		
		// Save font weight
		DWORD dwWeight = (DWORD)pLogFont->lfWeight;
		RegSetValueEx(hKey, FONT_WEIGHT, 0, REG_DWORD, (LPBYTE)&dwWeight, sizeof(DWORD));
		
		// Save font italic
		DWORD dwItalic = (DWORD)pLogFont->lfItalic;
		RegSetValueEx(hKey, FONT_ITALIC, 0, REG_DWORD, (LPBYTE)&dwItalic, sizeof(DWORD));
		
		// Save font underline
		DWORD dwUnderline = (DWORD)pLogFont->lfUnderline;
		RegSetValueEx(hKey, FONT_UNDERLINE, 0, REG_DWORD, (LPBYTE)&dwUnderline, sizeof(DWORD));
		
		// Save font strikeout
		DWORD dwStrikeOut = (DWORD)pLogFont->lfStrikeOut;
		RegSetValueEx(hKey, FONT_STRIKEOUT, 0, REG_DWORD, (LPBYTE)&dwStrikeOut, sizeof(DWORD));
		
		// Save font charset
		DWORD dwCharSet = (DWORD)pLogFont->lfCharSet;
		RegSetValueEx(hKey, FONT_CHARSET, 0, REG_DWORD, (LPBYTE)&dwCharSet, sizeof(DWORD));
		
		RegCloseKey(hKey);
	}
}

// Create a registry key
BOOL CreateRegistryKey(HKEY hKey, LPCTSTR lpSubKey) {
	HKEY hNewKey;
	DWORD dwDisposition;
	
	LONG result = RegCreateKeyEx(
		hKey,
		lpSubKey,
		0,
		NULL,
		REG_OPTION_NON_VOLATILE,
		KEY_ALL_ACCESS,
		NULL,
		&hNewKey,
		&dwDisposition
	);
	
	if (result == ERROR_SUCCESS) {
		RegCloseKey(hNewKey);
		return TRUE;
	}
	
	return FALSE;
}

// Delete a registry key
BOOL DeleteRegistryKey(HKEY hKey, LPCTSTR lpSubKey) {
	return (RegDeleteKey(hKey, lpSubKey) == ERROR_SUCCESS);
}

// Read a string value from registry
BOOL ReadRegistryString(HKEY hKey, LPCTSTR lpSubKey, LPCTSTR lpValueName, LPTSTR lpData, DWORD dwSize) {
	HKEY hOpenKey;
	DWORD dwType;
	BOOL result = FALSE;
	
	if (RegOpenKeyEx(hKey, lpSubKey, 0, KEY_READ, &hOpenKey) == ERROR_SUCCESS) {
		if (RegQueryValueEx(hOpenKey, lpValueName, NULL, &dwType, (LPBYTE)lpData, &dwSize) == ERROR_SUCCESS) {
			if (dwType == REG_SZ || dwType == REG_EXPAND_SZ) {
				result = TRUE;
			}
		}
		RegCloseKey(hOpenKey);
	}
	
	return result;
}

// Write a string value to registry
BOOL WriteRegistryString(HKEY hKey, LPCTSTR lpSubKey, LPCTSTR lpValueName, LPCTSTR lpData) {
	HKEY hOpenKey;
	DWORD dwDisposition;
	BOOL result = FALSE;
	
	if (RegCreateKeyEx(hKey, lpSubKey, 0, NULL, REG_OPTION_NON_VOLATILE, KEY_WRITE, NULL, &hOpenKey, &dwDisposition) == ERROR_SUCCESS) {
		if (RegSetValueEx(hOpenKey, lpValueName, 0, REG_SZ, (LPBYTE)lpData, (_tcslen(lpData) + 1) * sizeof(TCHAR)) == ERROR_SUCCESS) {
			result = TRUE;
		}
		RegCloseKey(hOpenKey);
	}
	
	return result;
}

// Read a DWORD value from registry
BOOL ReadRegistryDWORD(HKEY hKey, LPCTSTR lpSubKey, LPCTSTR lpValueName, DWORD* pValue) {
	HKEY hOpenKey;
	DWORD dwType, dwSize = sizeof(DWORD);
	BOOL result = FALSE;
	
	if (RegOpenKeyEx(hKey, lpSubKey, 0, KEY_READ, &hOpenKey) == ERROR_SUCCESS) {
		if (RegQueryValueEx(hOpenKey, lpValueName, NULL, &dwType, (LPBYTE)pValue, &dwSize) == ERROR_SUCCESS) {
			if (dwType == REG_DWORD) {
				result = TRUE;
			}
		}
		RegCloseKey(hOpenKey);
	}
	
	return result;
}

// Write a DWORD value to registry
BOOL WriteRegistryDWORD(HKEY hKey, LPCTSTR lpSubKey, LPCTSTR lpValueName, DWORD value) {
	HKEY hOpenKey;
	DWORD dwDisposition;
	BOOL result = FALSE;
	
	if (RegCreateKeyEx(hKey, lpSubKey, 0, NULL, REG_OPTION_NON_VOLATILE, KEY_WRITE, NULL, &hOpenKey, &dwDisposition) == ERROR_SUCCESS) {
		if (RegSetValueEx(hOpenKey, lpValueName, 0, REG_DWORD, (LPBYTE)&value, sizeof(DWORD)) == ERROR_SUCCESS) {
			result = TRUE;
		}
		RegCloseKey(hOpenKey);
	}
	
	return result;
}
