#include "stdafx.h"
#include "Print.h"

// Global variables for print settings
static PRINTDLG g_printDlg = {0};
static PAGESETUPDLG g_pageSetupDlg = {0};
static BOOL g_bPrintInitialized = FALSE;

// Initialize print settings
VOID InitializePrintSettings() {
	if (g_bPrintInitialized) return;
	
	// Initialize the print dialog structure
	ZeroMemory(&g_printDlg, sizeof(g_printDlg));
	g_printDlg.lStructSize = sizeof(g_printDlg);
	g_printDlg.Flags = PD_RETURNDC | PD_ALLPAGES | PD_USEDEVMODECOPIES | PD_NOPAGENUMS | PD_HIDEPRINTTOFILE | PD_NOSELECTION;
	
	// Initialize the page setup dialog structure
	ZeroMemory(&g_pageSetupDlg, sizeof(g_pageSetupDlg));
	g_pageSetupDlg.lStructSize = sizeof(g_pageSetupDlg);
	g_pageSetupDlg.Flags = PSD_MARGINS | PSD_INTHOUSANDTHSOFINCHES;
	g_pageSetupDlg.rtMargin.left = 1000;    // 1 inch
	g_pageSetupDlg.rtMargin.top = 1000;     // 1 inch
	g_pageSetupDlg.rtMargin.right = 1000;   // 1 inch
	g_pageSetupDlg.rtMargin.bottom = 1000;  // 1 inch
	
	g_bPrintInitialized = TRUE;
}

// Display the print dialog and execute printing
VOID ShowPrintDialog(HWND hWnd, HWND hEdit) {
	InitializePrintSettings();
	
	// Check if there is text content
	int textLength = GetWindowTextLength(hEdit);
	if (textLength == 0) {
		MessageBox(hWnd, _T("No text to print."), _T("Print"), MB_OK | MB_ICONINFORMATION);
		return;
	}
	
	g_printDlg.hwndOwner = hWnd;
	
	// Display the print dialog
	if (PrintDlg(&g_printDlg)) {
		HDC hPrinterDC = g_printDlg.hDC;
		
		if (hPrinterDC) {
			// Execute printing
			if (!PrintTextDocument(hWnd, hEdit, hPrinterDC)) {
				MessageBox(hWnd, _T("Print failed."), _T("Print Error"), MB_OK | MB_ICONERROR);
			}
			
			// Clean up the printer DC
			DeleteDC(hPrinterDC);
		}
	}
}

// Display the page setup dialog
VOID ShowPageSetupDialog(HWND hWnd) {
	InitializePrintSettings();
	
	g_pageSetupDlg.hwndOwner = hWnd;
	
	// Display the page setup dialog
	if (PageSetupDlg(&g_pageSetupDlg)) {
		// Page setup updated
		MessageBox(hWnd, _T("Page setup updated."), _T("Page Setup"), MB_OK | MB_ICONINFORMATION);
	}
}

// Calculate text metrics and page layout
VOID CalculateTextMetrics(HDC hDC, TEXTMETRIC* tm, int* linesPerPage, int* printableWidth) {
	// Set the print font first
	HFONT hPrintFont = CreateFont(
		-MulDiv(14, GetDeviceCaps(hDC, LOGPIXELSY), 72), // 14-point font, scaled by printer DPI
		0,                       // Automatic width
		0,                       // No rotation
		0,                       // No slant
		FW_NORMAL,              // Normal weight
		FALSE,                  // Not italic
		FALSE,                  // No underline
		FALSE,                  // No strikethrough
		DEFAULT_CHARSET,        // Default character set
		OUT_DEFAULT_PRECIS,     // Output precision
		CLIP_DEFAULT_PRECIS,    // Clipping precision
		CLEARTYPE_QUALITY,      // High quality
		DEFAULT_PITCH | FF_MODERN, // Font family
		_T("Segoe UI")          // Font name
	);
	
	HFONT hOldFont = NULL;
	if (hPrintFont) {
		hOldFont = (HFONT)SelectObject(hDC, hPrintFont);
	}
	
	GetTextMetrics(hDC, tm);
	
	// Restore the original font
	if (hOldFont) {
		SelectObject(hDC, hOldFont);
	}
	if (hPrintFont) {
		DeleteObject(hPrintFont);
	}
	
	int pageWidth = GetDeviceCaps(hDC, HORZRES);
	int pageHeight = GetDeviceCaps(hDC, VERTRES);
	
	// Calculate margins based on printer DPI (approximately 0.75 inches)
	int dpiX = GetDeviceCaps(hDC, LOGPIXELSX);
	int dpiY = GetDeviceCaps(hDC, LOGPIXELSY);
	int leftMargin = MulDiv(750, dpiX, 1000);   // 0.75 inches
	int topMargin = MulDiv(750, dpiY, 1000);    // 0.75 inches
	int rightMargin = leftMargin;
	int bottomMargin = topMargin;
	
	*printableWidth = pageWidth - leftMargin - rightMargin;
	int printableHeight = pageHeight - topMargin - bottomMargin;
	
	// Use actual line height to calculate lines per page
	*linesPerPage = printableHeight / tm->tmHeight;
	
	// Ensure reasonable minimum values
	if (*printableWidth < 200) *printableWidth = 200;
	if (*linesPerPage < 5) *linesPerPage = 5;
}

// Calculate the number of characters a text line can fit within a specified width
int CalculateLineLength(HDC hDC, LPCTSTR text, int maxWidth) {
	if (!text || maxWidth <= 0) return 0;
	
	int textLen = _tcslen(text);
	SIZE textSize;
	
	// Use binary search to find the maximum number of characters that can fit
	int left = 0, right = textLen;
	int result = 0;
	
	while (left <= right) {
		int mid = (left + right) / 2;
		
		if (GetTextExtentPoint32(hDC, text, mid, &textSize)) {
			if (textSize.cx <= maxWidth) {
				result = mid;
				left = mid + 1;
			} else {
				right = mid - 1;
			}
		} else {
			break;
		}
	}
	
	return result;
}

// Print a text page
BOOL PrintTextPage(HDC hDC, HWND hEdit, int startLine, int endLine, int pageNumber) {
	TEXTMETRIC tm;
	int linesPerPage, printableWidth;
	CalculateTextMetrics(hDC, &tm, &linesPerPage, &printableWidth);
	
	// Get all text from the edit control
	int totalTextLength = GetWindowTextLength(hEdit);
	if (totalTextLength == 0) return FALSE;
	
	LPTSTR allText = (LPTSTR)LocalAlloc(LMEM_ZEROINIT, (totalTextLength + 1) * sizeof(TCHAR));
	if (!allText) return FALSE;
	
	GetWindowText(hEdit, allText, totalTextLength + 1);
	
	// Analyze text and split into lines
	LPTSTR* displayLines = (LPTSTR*)LocalAlloc(LMEM_ZEROINIT, linesPerPage * sizeof(LPTSTR));
	if (!displayLines) {
		LocalFree(allText);
		return FALSE;
	}
	
	int currentOriginalLine = 0;
	int displayLineCount = 0;
	LPTSTR lineStart = allText;
	LPTSTR textPos = allText;
	
	// Skip to the starting line
	while (currentOriginalLine < startLine && *textPos) {
		if (*textPos == _T('\r') && *(textPos + 1) == _T('\n')) {
			currentOriginalLine++;
			textPos += 2;
			lineStart = textPos;
		} else if (*textPos == _T('\n')) {
			currentOriginalLine++;
			textPos++;
			lineStart = textPos;
		} else {
			textPos++;
		}
	}
	
	// Process lines to be printed, considering automatic line wrapping
	while (currentOriginalLine <= endLine && displayLineCount < linesPerPage && *textPos) {
		// Find the end position of the current line
		LPTSTR lineEnd = textPos;
		while (*lineEnd && *lineEnd != _T('\r') && *lineEnd != _T('\n')) {
			lineEnd++;
		}
		
		// Calculate the length of the current original line
		int originalLineLength = lineEnd - lineStart;
		
		if (originalLineLength == 0) {
			// Empty line
			displayLines[displayLineCount] = (LPTSTR)LocalAlloc(LMEM_ZEROINIT, sizeof(TCHAR));
			if (displayLines[displayLineCount]) {
				displayLines[displayLineCount][0] = _T('\0');
				displayLineCount++;
			}
		} else {
			// Non-empty line, may need to split into multiple display lines
			LPTSTR currentPos = lineStart;
			
			while (currentPos < lineEnd && displayLineCount < linesPerPage) {
				// Calculate the number of characters that can fit within the current width
				int remainingChars = lineEnd - currentPos;
				int charsInThisLine = CalculateLineLength(hDC, currentPos, printableWidth);
				
				if (charsInThisLine == 0 && remainingChars > 0) {
					// At least one character must be displayed, even if it exceeds the width
					charsInThisLine = 1;
				}
				
				if (charsInThisLine > remainingChars) {
					charsInThisLine = remainingChars;
				}
				
				// Allocate and copy the text for this display line
				displayLines[displayLineCount] = (LPTSTR)LocalAlloc(LMEM_ZEROINIT, (charsInThisLine + 1) * sizeof(TCHAR));
				if (displayLines[displayLineCount]) {
					_tcsncpy_s(displayLines[displayLineCount], charsInThisLine + 1, currentPos, charsInThisLine);
					displayLineCount++;
					currentPos += charsInThisLine;
				} else {
					break;
				}
			}
		}
		
		// Move to the next original line
		currentOriginalLine++;
		if (*lineEnd == _T('\r') && *(lineEnd + 1) == _T('\n')) {
			textPos = lineEnd + 2;
		} else if (*lineEnd == _T('\n')) {
			textPos = lineEnd + 1;
		} else {
			break;
		}
		lineStart = textPos;
	}
	
	// Set the print font (ensure consistency with metrics calculation)
	HFONT hPrintFont = CreateFont(
		-MulDiv(14, GetDeviceCaps(hDC, LOGPIXELSY), 72), // 14-point font, scaled by printer DPI
		0,                       // Automatic width
		0,                       // No rotation
		0,                       // No slant
		FW_NORMAL,              // Normal weight
		FALSE,                  // Not italic
		FALSE,                  // No underline
		FALSE,                  // No strikethrough
		DEFAULT_CHARSET,        // Default character set
		OUT_DEFAULT_PRECIS,     // Output precision
		CLIP_DEFAULT_PRECIS,    // Clipping precision
		CLEARTYPE_QUALITY,      // High quality
		DEFAULT_PITCH | FF_MODERN, // Font family
		_T("Segoe UI")          // Font name
	);
	
	HFONT hOldFont = NULL;
	if (hPrintFont) {
		hOldFont = (HFONT)SelectObject(hDC, hPrintFont);
	}
	
	// Recalculate text metrics to ensure consistency
	TEXTMETRIC actualTm;
	GetTextMetrics(hDC, &actualTm);
	
	// Calculate page margins
	int dpiX = GetDeviceCaps(hDC, LOGPIXELSX);
	int dpiY = GetDeviceCaps(hDC, LOGPIXELSY);
	int leftMargin = MulDiv(750, dpiX, 1000);   // 0.75 inches
	int topMargin = MulDiv(750, dpiY, 1000);    // 0.75 inches
	
	// Print page content
	int yPos = topMargin; // Start from the correct top margin
	
	// Print header (page number)
	TCHAR pageHeader[64];
	_stprintf_s(pageHeader, 64, _T("Page %d"), pageNumber);
	TextOut(hDC, GetDeviceCaps(hDC, HORZRES) - MulDiv(1500, dpiX, 1000), MulDiv(500, dpiY, 1000), pageHeader, _tcslen(pageHeader));
	
	// Print text lines using actual line height
	for (int i = 0; i < displayLineCount; i++) {
		if (displayLines[i]) {
			TextOut(hDC, leftMargin, yPos, displayLines[i], _tcslen(displayLines[i]));
			yPos += actualTm.tmHeight; // Use actual line height, no extra spacing
			LocalFree(displayLines[i]);
		}
	}
	
	// Restore the original font and clean up
	if (hOldFont) {
		SelectObject(hDC, hOldFont);
	}
	if (hPrintFont) {
		DeleteObject(hPrintFont);
	}
	
	// Free memory
	LocalFree(displayLines);
	LocalFree(allText);
	
	return TRUE;
}

// Print the entire document
BOOL PrintTextDocument(HWND hWnd, HWND hEdit, HDC hPrinterDC) {
	DOCINFO docInfo;
	ZeroMemory(&docInfo, sizeof(docInfo));
	docInfo.cbSize = sizeof(docInfo);
	docInfo.lpszDocName = _T("MyNotePad Document");
	
	// Start the print job
	if (StartDoc(hPrinterDC, &docInfo) <= 0) {
		return FALSE;
	}
	
	// Calculate total pages
	TEXTMETRIC tm;
	int linesPerPage, printableWidth;
	CalculateTextMetrics(hPrinterDC, &tm, &linesPerPage, &printableWidth);
	
	// Calculate total lines
	int totalLines = SendMessage(hEdit, EM_GETLINECOUNT, 0, 0);
	int totalPages = (totalLines + linesPerPage - 1) / linesPerPage;
	
	if (totalPages == 0) totalPages = 1;
	
	// Print each page
	for (int page = 1; page <= totalPages; page++) {
		// Start a new page
		if (StartPage(hPrinterDC) <= 0) {
			EndDoc(hPrinterDC);
			return FALSE;
		}
		
		// Calculate the line range for this page
		int startLine = (page - 1) * linesPerPage;
		int endLine = min(startLine + linesPerPage - 1, totalLines - 1);
		
		// Print this page
		if (!PrintTextPage(hPrinterDC, hEdit, startLine, endLine, page)) {
			EndPage(hPrinterDC);
			EndDoc(hPrinterDC);
			return FALSE;
		}
		
		// End the page
		if (EndPage(hPrinterDC) <= 0) {
			EndDoc(hPrinterDC);
			return FALSE;
		}
	}
	
	// End the print job
	EndDoc(hPrinterDC);
	return TRUE;
}
