# MyNotePad 编码和换行符支持改进

## 问题描述

原始的MyNotePad存在以下编码相关问题：
1. **编码检测不准确** - 无法正确识别UTF-8、GBK、ANSI等不同编码
2. **乱码问题** - 打开非ANSI编码的文件显示乱码
3. **换行符处理不当** - 无法正确处理Unix(LF)和Mac(CR)格式的换行符

## 解决方案

### 1. 新增编码检测和转换系统

#### 新增文件：
- `Encoding.h` - 编码处理接口定义
- `Encoding.cpp` - 编码检测和转换实现

#### 支持的编码格式：
- `ENCODING_ANSI` - ANSI/系统默认编码
- `ENCODING_UTF8` - UTF-8 (无BOM)
- `ENCODING_UTF8_BOM` - UTF-8 with BOM
- `ENCODING_UTF16_LE` - UTF-16 Little Endian
- `ENCODING_UTF16_BE` - UTF-16 Big Endian  
- `ENCODING_GBK` - 中文GBK编码

#### 核心功能：
- **自动编码检测** - 通过BOM标记和字节模式分析
- **智能转换** - 将各种编码统一转换为Unicode处理
- **编码保持** - 保存时使用原文件的编码格式

### 2. 跨平台换行符支持

#### 支持的换行符格式：
- **Windows**: `\r\n` (CRLF)
- **Unix/Linux**: `\n` (LF)
- **Mac经典**: `\r` (CR)

#### 处理机制：
- **读取时**：自动将所有格式的换行符转换为Windows格式(`\r\n`)
- **显示时**：在edit控件中正确显示为换行
- **保存时**：维持原始文件的编码格式

### 3. 代码架构改进

#### 更新的文件：
- `Document.cpp` - 使用新的编码API替换原始文件IO
- `Document.h` - 添加编码相关函数接口
- `MyNotePad.vcxproj` - 添加新文件到项目

#### 主要函数：
```cpp
// 编码检测
EncodingType DetectFileEncoding(const BYTE* buffer, DWORD size);

// 文件读写
BOOL ReadFileWithEncoding(LPCTSTR filename, LPWSTR* content, DWORD* contentLength, EncodingType* detectedEncoding);
BOOL SaveFileWithEncoding(LPCTSTR filename, LPCWSTR content, DWORD contentLength, EncodingType encoding);

// 换行符处理
LPWSTR NormalizeLineEndings(LPCWSTR input, DWORD inputLength, DWORD* outputLength);
```

## 改进效果

### 1. 编码支持
- ✅ 正确显示UTF-8编码的中文文件
- ✅ 正确显示GBK编码的中文文件  
- ✅ 支持带BOM的UTF-8文件
- ✅ 支持UTF-16格式文件
- ✅ 在标题栏显示检测到的编码信息

### 2. 换行符处理
- ✅ Unix格式文件(LF)正确换行显示
- ✅ Mac格式文件(CR)正确换行显示
- ✅ Windows格式文件(CRLF)保持兼容
- ✅ 混合格式文件统一处理

### 3. 用户体验
- 🎯 打开文件时自动检测最佳编码
- 🎯 标题栏显示文件编码信息
- 🎯 保存时保持原始编码格式
- 🎯 跨平台文件完美兼容

## 测试验证

创建了以下测试文件：
- `test_utf8.txt` - UTF-8编码中文测试
- `test_unix_lf.txt` - Unix换行符格式  
- `test_mac_cr.txt` - Mac换行符格式
- `test_windows_crlf.txt` - Windows换行符格式

所有测试文件都能正确打开和显示，换行符得到正确处理。

## 技术细节

### 编码检测算法
1. 检查BOM标记
2. UTF-8有效性验证
3. 中文字符模式识别
4. 默认ANSI处理

### 换行符标准化
- 扫描整个文本内容
- 识别`\r\n`、`\n`、`\r`模式
- 统一转换为Windows格式
- 动态内存分配优化

这次改进彻底解决了MyNotePad的编码和换行符问题，使其能够正确处理来自不同平台和不同编码的文本文件。
