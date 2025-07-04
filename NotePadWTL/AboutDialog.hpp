#pragma once

#include "Resource.h"
#include "pch.h"

// Link required libraries
#pragma comment(lib, "version.lib")

// Forward declarations and type definitions
#ifndef NTSTATUS
typedef LONG NTSTATUS;
#endif

typedef NTSTATUS(WINAPI* PFN_RtlGetVersion)(PRTL_OSVERSIONINFOW);
typedef BOOL(WINAPI* PFN_GetProductInfo)(DWORD, DWORD, DWORD, DWORD, PDWORD);

// About Dialog Class - WTL Style
class CAboutDialog : public CDialogImpl<CAboutDialog>
{
public:
	enum { IDD = IDD_ABOUTBOX };

	BEGIN_MSG_MAP(CAboutDialog)
		MESSAGE_HANDLER(WM_INITDIALOG, OnInitDialog)
		COMMAND_ID_HANDLER(IDOK, OnCloseCmd)
		COMMAND_ID_HANDLER(IDCANCEL, OnCloseCmd)
	END_MSG_MAP()

	// Message handlers
	LRESULT OnInitDialog(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/)
	{
		CenterWindow(GetParent());
		UpdateAboutDialogInfo();
		return TRUE; // Let the system set the focus
	}

	LRESULT OnCloseCmd(WORD /*wNotifyCode*/, WORD wID, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
	{
		EndDialog(wID);
		return 0;
	}

	// Update About dialog information
	VOID UpdateAboutDialogInfo()
	{
		// Get and set Windows version information
		TCHAR szWindowsVersion[256];
		GetWindowsVersionString(szWindowsVersion, 256);
		SetDlgItemText(IDC_ABOUT_WIN_VER, szWindowsVersion);

		// Get and set CPU information
		TCHAR szCPUInfo[256];
		GetCPUInfo(szCPUInfo, 256);
		SetDlgItemText(IDC_ABOUT_CPU_INFO, szCPUInfo);

		// Get and set memory information
		TCHAR szMemoryInfo[256];
		GetMemoryInfo(szMemoryInfo, 256);
		SetDlgItemText(IDC_ABOUT_MEMORY_INFO, szMemoryInfo);
	}

private:

	// Static helper methods for system information retrieval
	static VOID GetProductVersionInfo(LPTSTR lpszProductInfo, DWORD dwSize)
	{
		if (!lpszProductInfo || dwSize == 0) 
			return;

		// Initialize to empty string
		lpszProductInfo[0] = _T('\0');

		// Use GetProductInfo API (Vista+)
		HMODULE hKernel32 = GetModuleHandle(_T("kernel32.dll"));
		if (hKernel32)
		{
			PFN_GetProductInfo pfnGetProductInfo = 
				reinterpret_cast<PFN_GetProductInfo>(GetProcAddress(hKernel32, "GetProductInfo"));
			if (pfnGetProductInfo)
			{
				DWORD dwProductType = 0;
				if (pfnGetProductInfo(6, 0, 0, 0, &dwProductType)) // Use Windows Vista as baseline
				{
					switch (dwProductType)
					{
					case 0x00000001: // PRODUCT_ULTIMATE
						_tcscpy_s(lpszProductInfo, dwSize, _T(" Ultimate"));
						break;
					case 0x00000002: // PRODUCT_HOME_BASIC
						_tcscpy_s(lpszProductInfo, dwSize, _T(" Home Basic"));
						break;
					case 0x00000003: // PRODUCT_HOME_PREMIUM
						_tcscpy_s(lpszProductInfo, dwSize, _T(" Home Premium"));
						break;
					case 0x00000030: // PRODUCT_PROFESSIONAL
						_tcscpy_s(lpszProductInfo, dwSize, _T(" Professional"));
						break;
					case 0x00000048: // PRODUCT_PROFESSIONAL_N
						_tcscpy_s(lpszProductInfo, dwSize, _T(" Professional N"));
						break;
					case 0x00000031: // PRODUCT_PROFESSIONAL_WMC
						_tcscpy_s(lpszProductInfo, dwSize, _T(" Professional with Media Center"));
						break;
					case 0x00000101: // PRODUCT_HOME
						_tcscpy_s(lpszProductInfo, dwSize, _T(" Home"));
						break;
					case 0x00000103: // PRODUCT_HOME_N
						_tcscpy_s(lpszProductInfo, dwSize, _T(" Home N"));
						break;
					case 0x00000104: // PRODUCT_HOME_SINGLE_LANGUAGE
						_tcscpy_s(lpszProductInfo, dwSize, _T(" Home Single Language"));
						break;
					case 0x00000006: // PRODUCT_BUSINESS
						_tcscpy_s(lpszProductInfo, dwSize, _T(" Business"));
						break;
					case 0x00000004: // PRODUCT_ENTERPRISE
						_tcscpy_s(lpszProductInfo, dwSize, _T(" Enterprise"));
						break;
					case 0x00000161: // PRODUCT_PRO_WORKSTATION
						_tcscpy_s(lpszProductInfo, dwSize, _T(" Pro for Workstations"));
						break;
					case 0x00000162: // PRODUCT_PRO_WORKSTATION_N
						_tcscpy_s(lpszProductInfo, dwSize, _T(" Pro for Workstations N"));
						break;
					default:
						// For unknown product types, don't add any information
						break;
					}
				}
			}
		}
	}

	static VOID ProcessVersionInfo(const RTL_OSVERSIONINFOW& osvi, LPTSTR lpszVersionString, DWORD dwBufferSize)
	{
		// Determine Windows version based on version number
		if (osvi.dwMajorVersion == 10)
		{
			if (osvi.dwBuildNumber >= 22000)
			{
				_tcscpy_s(lpszVersionString, dwBufferSize, _T("Windows 11"));
			}
			else
			{
				_tcscpy_s(lpszVersionString, dwBufferSize, _T("Windows 10"));
			}

			// Add build number
			TCHAR szBuild[32];
			_stprintf_s(szBuild, 32, _T(" (Build %d)"), osvi.dwBuildNumber);
			_tcscat_s(lpszVersionString, dwBufferSize, szBuild);
		}
		else if (osvi.dwMajorVersion == 6)
		{
			switch (osvi.dwMinorVersion)
			{
			case 3:
				_tcscpy_s(lpszVersionString, dwBufferSize, _T("Windows 8.1"));
				break;
			case 2:
				_tcscpy_s(lpszVersionString, dwBufferSize, _T("Windows 8"));
				break;
			case 1:
				_tcscpy_s(lpszVersionString, dwBufferSize, _T("Windows 7"));
				break;
			case 0:
				_tcscpy_s(lpszVersionString, dwBufferSize, _T("Windows Vista"));
				break;
			default:
				_stprintf_s(lpszVersionString, dwBufferSize, _T("Windows NT %d.%d"),
					osvi.dwMajorVersion, osvi.dwMinorVersion);
				break;
			}
		}
		else if (osvi.dwMajorVersion == 5)
		{
			switch (osvi.dwMinorVersion)
			{
			case 2:
				_tcscpy_s(lpszVersionString, dwBufferSize, _T("Windows Server 2003"));
				break;
			case 1:
				_tcscpy_s(lpszVersionString, dwBufferSize, _T("Windows XP"));
				break;
			case 0:
				_tcscpy_s(lpszVersionString, dwBufferSize, _T("Windows 2000"));
				break;
			default:
				_stprintf_s(lpszVersionString, dwBufferSize, _T("Windows NT %d.%d"),
					osvi.dwMajorVersion, osvi.dwMinorVersion);
				break;
			}
		}
		else
		{
			// For unknown versions, display the version number
			_stprintf_s(lpszVersionString, dwBufferSize, _T("Windows NT %d.%d (Build %d)"),
				osvi.dwMajorVersion, osvi.dwMinorVersion, osvi.dwBuildNumber);
		}
	}

	static VOID GetVersionFromFileInfo(LPTSTR lpszVersionString, DWORD dwBufferSize)
	{
		TCHAR szSystemPath[MAX_PATH];
		GetSystemDirectory(szSystemPath, MAX_PATH);
		_tcscat_s(szSystemPath, MAX_PATH, _T("\\kernel32.dll"));

		DWORD dwHandle = 0;
		DWORD dwSize = GetFileVersionInfoSize(szSystemPath, &dwHandle);

		if (dwSize > 0)
		{
			LPBYTE pVersionInfo = static_cast<LPBYTE>(LocalAlloc(LMEM_ZEROINIT, dwSize));
			if (pVersionInfo)
			{
				if (GetFileVersionInfo(szSystemPath, dwHandle, dwSize, pVersionInfo))
				{
					VS_FIXEDFILEINFO* pFileInfo = NULL;
					UINT uLen = 0;

					if (VerQueryValue(pVersionInfo, _T("\\"), reinterpret_cast<LPVOID*>(&pFileInfo), &uLen) && pFileInfo)
					{
						DWORD dwMajorVersion = HIWORD(pFileInfo->dwProductVersionMS);
						DWORD dwMinorVersion = LOWORD(pFileInfo->dwProductVersionMS);
						DWORD dwBuildNumber = HIWORD(pFileInfo->dwProductVersionLS);

						RTL_OSVERSIONINFOW osvi = { 0 };
						osvi.dwMajorVersion = dwMajorVersion;
						osvi.dwMinorVersion = dwMinorVersion;
						osvi.dwBuildNumber = dwBuildNumber;
						ProcessVersionInfo(osvi, lpszVersionString, dwBufferSize);
					}
				}
				LocalFree(pVersionInfo);
			}
		}

		// Add product version information
		TCHAR szProductInfo[64];
		GetProductVersionInfo(szProductInfo, 64);
		if (_tcslen(szProductInfo) > 0)
		{
			_tcscat_s(lpszVersionString, dwBufferSize, szProductInfo);
		}
	}

	static VOID GetWindowsVersionString(LPTSTR lpszVersionString, DWORD dwBufferSize)
	{
		if (!lpszVersionString || dwBufferSize == 0) 
			return;

		// Initialize to default value
		_tcscpy_s(lpszVersionString, dwBufferSize, _T("Windows"));

		// First try using RtlGetVersion (more accurate, not affected by application compatibility)
		HMODULE hNtdll = GetModuleHandle(_T("ntdll.dll"));
		if (hNtdll)
		{
			PFN_RtlGetVersion pfnRtlGetVersion = 
				reinterpret_cast<PFN_RtlGetVersion>(GetProcAddress(hNtdll, "RtlGetVersion"));
			if (pfnRtlGetVersion)
			{
				RTL_OSVERSIONINFOW osvi = { 0 };
				osvi.dwOSVersionInfoSize = sizeof(osvi);

				if (pfnRtlGetVersion(&osvi) == 0) // STATUS_SUCCESS
				{
					ProcessVersionInfo(osvi, lpszVersionString, dwBufferSize);
					
					// Add product version information
					TCHAR szProductInfo[64];
					GetProductVersionInfo(szProductInfo, 64);
					if (_tcslen(szProductInfo) > 0)
					{
						_tcscat_s(lpszVersionString, dwBufferSize, szProductInfo);
					}
					return; // Successfully got version info
				}
			}
		}

		// Fallback to file version info method
		GetVersionFromFileInfo(lpszVersionString, dwBufferSize);
	}

	static VOID GetCPUInfo(LPTSTR lpszCPUInfo, DWORD dwBufferSize)
	{
		if (!lpszCPUInfo || dwBufferSize == 0) 
			return;

		// Initialize to default value
		_tcscpy_s(lpszCPUInfo, dwBufferSize, _T("Unknown CPU"));

		// Get CPU information from registry
		HKEY hKey;
		if (RegOpenKeyEx(HKEY_LOCAL_MACHINE,
			_T("HARDWARE\\DESCRIPTION\\System\\CentralProcessor\\0"),
			0, KEY_READ, &hKey) == ERROR_SUCCESS)
		{
			TCHAR szProcessorName[256] = { 0 };
			DWORD dwType = REG_SZ;
			DWORD dwSize = sizeof(szProcessorName);

			if (RegQueryValueEx(hKey, _T("ProcessorNameString"), NULL, &dwType,
				reinterpret_cast<LPBYTE>(szProcessorName), &dwSize) == ERROR_SUCCESS)
			{
				// Remove extra spaces
				LPTSTR pStart = szProcessorName;
				while (*pStart == _T(' ')) 
					pStart++;

				LPTSTR pEnd = pStart + _tcslen(pStart) - 1;
				while (pEnd > pStart && *pEnd == _T(' '))
				{
					*pEnd = _T('\0');
					pEnd--;
				}

				_tcscpy_s(lpszCPUInfo, dwBufferSize, pStart);
			}

			RegCloseKey(hKey);
		}

		// Get system information to add core count
		SYSTEM_INFO sysInfo;
		GetSystemInfo(&sysInfo);

		// Add core count information
		TCHAR szCoreInfo[64];
		_stprintf_s(szCoreInfo, 64, _T(" (%d Cores)"), sysInfo.dwNumberOfProcessors);
		_tcscat_s(lpszCPUInfo, dwBufferSize, szCoreInfo);
	}

	static VOID GetMemoryInfo(LPTSTR lpszMemoryInfo, DWORD dwBufferSize)
	{
		if (!lpszMemoryInfo || dwBufferSize == 0) 
			return;

		// Initialize to default value
		_tcscpy_s(lpszMemoryInfo, dwBufferSize, _T("Unknown Memory"));

		MEMORYSTATUSEX memStatus;
		memStatus.dwLength = sizeof(memStatus);

		if (GlobalMemoryStatusEx(&memStatus))
		{
			// Calculate total memory (MB/GB)
			ULONGLONG totalMemoryMB = memStatus.ullTotalPhys / (1024 * 1024);
			ULONGLONG availableMemoryMB = memStatus.ullAvailPhys / (1024 * 1024);
			ULONGLONG usedMemoryMB = totalMemoryMB - availableMemoryMB;

			// Format memory information
			if (totalMemoryMB >= 1024)
			{
				// Display as GB
				double totalGB = static_cast<double>(totalMemoryMB) / 1024.0;
				double usedGB = static_cast<double>(usedMemoryMB) / 1024.0;
				_stprintf_s(lpszMemoryInfo, dwBufferSize,
					_T("%.1f GB Total Memory (%.1f GB Used, %d%% Usage)"),
					totalGB, usedGB, memStatus.dwMemoryLoad);
			}
			else
			{
				// Display as MB
				_stprintf_s(lpszMemoryInfo, dwBufferSize,
					_T("%llu MB Total Memory (%llu MB Used, %d%% Usage)"),
					totalMemoryMB, usedMemoryMB, memStatus.dwMemoryLoad);
			}
		}
	}
};