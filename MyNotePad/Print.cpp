#include "stdafx.h"
#include "Print.h"

// 打印设置的全局变量
static PRINTDLG g_printDlg = {0};
static PAGESETUPDLG g_pageSetupDlg = {0};
static BOOL g_bPrintInitialized = FALSE;

// 初始化打印设置
VOID InitializePrintSettings() {
	if (g_bPrintInitialized) return;
	
	// 初始化打印对话框结构
	ZeroMemory(&g_printDlg, sizeof(g_printDlg));
	g_printDlg.lStructSize = sizeof(g_printDlg);
	g_printDlg.Flags = PD_RETURNDC | PD_ALLPAGES | PD_USEDEVMODECOPIES | PD_NOPAGENUMS | PD_HIDEPRINTTOFILE | PD_NOSELECTION;
	
	// 初始化页面设置对话框结构
	ZeroMemory(&g_pageSetupDlg, sizeof(g_pageSetupDlg));
	g_pageSetupDlg.lStructSize = sizeof(g_pageSetupDlg);
	g_pageSetupDlg.Flags = PSD_MARGINS | PSD_INTHOUSANDTHSOFINCHES;
	g_pageSetupDlg.rtMargin.left = 1000;    // 1 inch
	g_pageSetupDlg.rtMargin.top = 1000;     // 1 inch
	g_pageSetupDlg.rtMargin.right = 1000;   // 1 inch
	g_pageSetupDlg.rtMargin.bottom = 1000;  // 1 inch
	
	g_bPrintInitialized = TRUE;
}

// 显示打印对话框并执行打印
VOID ShowPrintDialog(HWND hWnd, HWND hEdit) {
	InitializePrintSettings();
	
	// 检查是否有文本内容
	int textLength = GetWindowTextLength(hEdit);
	if (textLength == 0) {
		MessageBox(hWnd, _T("No text to print."), _T("Print"), MB_OK | MB_ICONINFORMATION);
		return;
	}
	
	g_printDlg.hwndOwner = hWnd;
	
	// 显示打印对话框
	if (PrintDlg(&g_printDlg)) {
		HDC hPrinterDC = g_printDlg.hDC;
		
		if (hPrinterDC) {
			// 执行打印
			if (!PrintTextDocument(hWnd, hEdit, hPrinterDC)) {
				MessageBox(hWnd, _T("Print failed."), _T("Print Error"), MB_OK | MB_ICONERROR);
			}
			
			// 清理打印机DC
			DeleteDC(hPrinterDC);
		}
	}
}

// 显示页面设置对话框
VOID ShowPageSetupDialog(HWND hWnd) {
	InitializePrintSettings();
	
	g_pageSetupDlg.hwndOwner = hWnd;
	
	// 显示页面设置对话框
	if (PageSetupDlg(&g_pageSetupDlg)) {
		// 页面设置已更新
		MessageBox(hWnd, _T("Page setup updated."), _T("Page Setup"), MB_OK | MB_ICONINFORMATION);
	}
}

// 计算文本度量和页面布局
VOID CalculateTextMetrics(HDC hDC, TEXTMETRIC* tm, int* linesPerPage, int* printableWidth) {
	// 先设置打印字体
	HFONT hPrintFont = CreateFont(
		-MulDiv(14, GetDeviceCaps(hDC, LOGPIXELSY), 72), // 14磅字体，按打印机DPI缩放
		0,                       // 宽度自动
		0,                       // 不旋转
		0,                       // 不倾斜
		FW_NORMAL,              // 正常粗细
		FALSE,                  // 不斜体
		FALSE,                  // 不下划线
		FALSE,                  // 不删除线
		DEFAULT_CHARSET,        // 默认字符集
		OUT_DEFAULT_PRECIS,     // 输出精度
		CLIP_DEFAULT_PRECIS,    // 裁剪精度
		CLEARTYPE_QUALITY,      // 高质量
		DEFAULT_PITCH | FF_MODERN, // 字体族
		_T("Segoe UI")          // 字体名称
	);
	
	HFONT hOldFont = NULL;
	if (hPrintFont) {
		hOldFont = (HFONT)SelectObject(hDC, hPrintFont);
	}
	
	GetTextMetrics(hDC, tm);
	
	// 恢复原字体
	if (hOldFont) {
		SelectObject(hDC, hOldFont);
	}
	if (hPrintFont) {
		DeleteObject(hPrintFont);
	}
	
	int pageWidth = GetDeviceCaps(hDC, HORZRES);
	int pageHeight = GetDeviceCaps(hDC, VERTRES);
	
	// 按打印机DPI计算页边距（约0.75英寸）
	int dpiX = GetDeviceCaps(hDC, LOGPIXELSX);
	int dpiY = GetDeviceCaps(hDC, LOGPIXELSY);
	int leftMargin = MulDiv(750, dpiX, 1000);   // 0.75英寸
	int topMargin = MulDiv(750, dpiY, 1000);    // 0.75英寸
	int rightMargin = leftMargin;
	int bottomMargin = topMargin;
	
	*printableWidth = pageWidth - leftMargin - rightMargin;
	int printableHeight = pageHeight - topMargin - bottomMargin;
	
	// 使用实际行高计算每页行数
	*linesPerPage = printableHeight / tm->tmHeight;
	
	// 确保合理的最小值
	if (*printableWidth < 200) *printableWidth = 200;
	if (*linesPerPage < 5) *linesPerPage = 5;
}

// 计算文本行在指定宽度内能容纳的字符数
int CalculateLineLength(HDC hDC, LPCTSTR text, int maxWidth) {
	if (!text || maxWidth <= 0) return 0;
	
	int textLen = _tcslen(text);
	SIZE textSize;
	
	// 使用二分查找找到最大能容纳的字符数
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

// 打印文本页面
BOOL PrintTextPage(HDC hDC, HWND hEdit, int startLine, int endLine, int pageNumber) {
	TEXTMETRIC tm;
	int linesPerPage, printableWidth;
	CalculateTextMetrics(hDC, &tm, &linesPerPage, &printableWidth);
	
	// 获取编辑控件中的所有文本
	int totalTextLength = GetWindowTextLength(hEdit);
	if (totalTextLength == 0) return FALSE;
	
	LPTSTR allText = (LPTSTR)LocalAlloc(LMEM_ZEROINIT, (totalTextLength + 1) * sizeof(TCHAR));
	if (!allText) return FALSE;
	
	GetWindowText(hEdit, allText, totalTextLength + 1);
	
	// 分析文本，按行分割
	LPTSTR* displayLines = (LPTSTR*)LocalAlloc(LMEM_ZEROINIT, linesPerPage * sizeof(LPTSTR));
	if (!displayLines) {
		LocalFree(allText);
		return FALSE;
	}
	
	int currentOriginalLine = 0;
	int displayLineCount = 0;
	LPTSTR lineStart = allText;
	LPTSTR textPos = allText;
	
	// 跳过到起始行
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
	
	// 处理要打印的行，考虑自动换行
	while (currentOriginalLine <= endLine && displayLineCount < linesPerPage && *textPos) {
		// 找到当前行的结束位置
		LPTSTR lineEnd = textPos;
		while (*lineEnd && *lineEnd != _T('\r') && *lineEnd != _T('\n')) {
			lineEnd++;
		}
		
		// 计算当前原始行的长度
		int originalLineLength = lineEnd - lineStart;
		
		if (originalLineLength == 0) {
			// 空行
			displayLines[displayLineCount] = (LPTSTR)LocalAlloc(LMEM_ZEROINIT, sizeof(TCHAR));
			if (displayLines[displayLineCount]) {
				displayLines[displayLineCount][0] = _T('\0');
				displayLineCount++;
			}
		} else {
			// 非空行，可能需要分成多个显示行
			LPTSTR currentPos = lineStart;
			
			while (currentPos < lineEnd && displayLineCount < linesPerPage) {
				// 计算在当前宽度下能容纳多少字符
				int remainingChars = lineEnd - currentPos;
				int charsInThisLine = CalculateLineLength(hDC, currentPos, printableWidth);
				
				if (charsInThisLine == 0 && remainingChars > 0) {
					// 至少要显示一个字符，即使超宽
					charsInThisLine = 1;
				}
				
				if (charsInThisLine > remainingChars) {
					charsInThisLine = remainingChars;
				}
				
				// 分配并复制这一显示行的文本
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
		
		// 移动到下一个原始行
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
	
	// 设置打印字体（确保与度量计算一致）
	HFONT hPrintFont = CreateFont(
		-MulDiv(14, GetDeviceCaps(hDC, LOGPIXELSY), 72), // 14磅字体，按打印机DPI缩放
		0,                       // 宽度自动
		0,                       // 不旋转
		0,                       // 不倾斜
		FW_NORMAL,              // 正常粗细
		FALSE,                  // 不斜体
		FALSE,                  // 不下划线
		FALSE,                  // 不删除线
		DEFAULT_CHARSET,        // 默认字符集
		OUT_DEFAULT_PRECIS,     // 输出精度
		CLIP_DEFAULT_PRECIS,    // 裁剪精度
		CLEARTYPE_QUALITY,      // 高质量
		DEFAULT_PITCH | FF_MODERN, // 字体族
		_T("Segoe UI")          // 字体名称
	);
	
	HFONT hOldFont = NULL;
	if (hPrintFont) {
		hOldFont = (HFONT)SelectObject(hDC, hPrintFont);
	}
	
	// 重新获取文本度量以确保一致性
	TEXTMETRIC actualTm;
	GetTextMetrics(hDC, &actualTm);
	
	// 计算页边距
	int dpiX = GetDeviceCaps(hDC, LOGPIXELSX);
	int dpiY = GetDeviceCaps(hDC, LOGPIXELSY);
	int leftMargin = MulDiv(750, dpiX, 1000);   // 0.75英寸
	int topMargin = MulDiv(750, dpiY, 1000);    // 0.75英寸
	
	// 打印页面内容
	int yPos = topMargin; // 从正确的顶部边距开始
	
	// 打印页眉（页码）
	TCHAR pageHeader[64];
	_stprintf_s(pageHeader, 64, _T("Page %d"), pageNumber);
	TextOut(hDC, GetDeviceCaps(hDC, HORZRES) - MulDiv(1500, dpiX, 1000), MulDiv(500, dpiY, 1000), pageHeader, _tcslen(pageHeader));
	
	// 打印文本行，使用实际的行高
	for (int i = 0; i < displayLineCount; i++) {
		if (displayLines[i]) {
			TextOut(hDC, leftMargin, yPos, displayLines[i], _tcslen(displayLines[i]));
			yPos += actualTm.tmHeight; // 使用实际的行高，没有额外间距
			LocalFree(displayLines[i]);
		}
	}
	
	// 恢复原字体并清理
	if (hOldFont) {
		SelectObject(hDC, hOldFont);
	}
	if (hPrintFont) {
		DeleteObject(hPrintFont);
	}
	
	// 清理内存
	LocalFree(displayLines);
	LocalFree(allText);
	
	return TRUE;
}

// 打印整个文档
BOOL PrintTextDocument(HWND hWnd, HWND hEdit, HDC hPrinterDC) {
	DOCINFO docInfo;
	ZeroMemory(&docInfo, sizeof(docInfo));
	docInfo.cbSize = sizeof(docInfo);
	docInfo.lpszDocName = _T("MyNotePad Document");
	
	// 开始打印作业
	if (StartDoc(hPrinterDC, &docInfo) <= 0) {
		return FALSE;
	}
	
	// 计算总页数
	TEXTMETRIC tm;
	int linesPerPage, printableWidth;
	CalculateTextMetrics(hPrinterDC, &tm, &linesPerPage, &printableWidth);
	
	// 计算总行数
	int totalLines = SendMessage(hEdit, EM_GETLINECOUNT, 0, 0);
	int totalPages = (totalLines + linesPerPage - 1) / linesPerPage;
	
	if (totalPages == 0) totalPages = 1;
	
	// 打印每一页
	for (int page = 1; page <= totalPages; page++) {
		// 开始新页
		if (StartPage(hPrinterDC) <= 0) {
			EndDoc(hPrinterDC);
			return FALSE;
		}
		
		// 计算这一页的行范围
		int startLine = (page - 1) * linesPerPage;
		int endLine = min(startLine + linesPerPage - 1, totalLines - 1);
		
		// 打印这一页
		if (!PrintTextPage(hPrinterDC, hEdit, startLine, endLine, page)) {
			EndPage(hPrinterDC);
			EndDoc(hPrinterDC);
			return FALSE;
		}
		
		// 结束页面
		if (EndPage(hPrinterDC) <= 0) {
			EndDoc(hPrinterDC);
			return FALSE;
		}
	}
	
	// 结束打印作业
	EndDoc(hPrinterDC);
	return TRUE;
}
