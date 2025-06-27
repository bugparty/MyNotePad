# MyNotePad

A modern Windows Notepad application with enhanced editing features and print support.

![Build Status](https://github.com/bugparty/MyNotePad/workflows/Build%20and%20Release%20MyNotePad/badge.svg)

![My Notepad Screenshot](./Screenshot01.png)

## 📋 Table of Contents

- [Features](#-features)
- [Downloads](#-downloads)
- [Build Instructions](#️-build-instructions)
- [Technical Details](#-technical-details)
- [Contributing](#-contributing)
- [License](#-license)
- [Acknowledgments](#acknowledgments)


## ✨ Features

**Fully implements all functionality of the classic Windows Notepad with modern enhancements.**

### 📝 Text Editing

- **Modern Editing Interface** - Uses Segoe UI font with white background and dark gray text
- **Multi-line Editing Support** - Proper handling of line breaks and Tab key support
- **Undo Functionality** - Support for undo/redo operations
- **Select All** - Ctrl+A keyboard shortcut and menu option

### 🔍 Find and Replace

- **Find Dialog** - Support for forward/backward search
- **Replace Functionality** - Single replace and replace all operations
- **Advanced Options** - Case sensitive and whole word matching

### 📄 File Operations

- **Multi-encoding Support** - Auto-detection of UTF-8, GBK, ANSI, UTF-16 and other encodings
- **Cross-platform Line Endings** - Support for Windows (CRLF), Unix (LF), and Mac (CR) line endings
- **Smart Save** - Preserves original file encoding format

### 🖨️ Print Features

- **Smart Font Scaling** - Automatically adjusts font size based on printer DPI
- **Mixed CJK and Latin Text Printing** - Correct handling of Chinese character widths
- **Automatic Pagination** - Smart calculation of page layout and pagination
- **Page Setup** - Support for margins and paper settings

## 📦 Downloads

### Download from Releases

Visit the [Releases page](https://github.com/bugparty/MyNotePad/releases) to download the latest version.

### Download from Artifacts

After each build, you can download the latest build results from the Actions page.

## 🏗️ Build Instructions

### Local Build

#### System Requirements

- Windows 10/11
- Visual Studio 2019/2022 (Community edition is sufficient)
- Windows SDK

#### Build Steps

```powershell
# Clone the repository
git clone https://github.com/bugparty/MyNotePad.git
cd MyNotePad
./build.ps1
```

### Automated Build (GitHub Actions)

The project is configured with GitHub Actions for automated builds:

- **Triggers**: Push to main/master branches, Pull Requests, or Release publishing
- **Build Environment**: Windows Server 2022
- **Output**: Release version exe files
- **Artifacts**: Each build uploads executable files to GitHub

#### Release Process

1. **Development Push**: Pushing code to main branch triggers build and uploads artifacts
2. **Create Release**: Creating git tags and releases automatically builds and attaches zip packages

```powershell
# Create version tag
git tag v1.0.0
git push origin v1.0.0

# Create Release on GitHub to trigger automatic build and file attachment
```

## 🔧 Technical Details

### Architecture

- **Language**: C++ (Visual C++)
- **UI Framework**: Win32 API
- **Character Encoding**: Unicode (UTF-16) internal processing
- **Compiler**: MSVC 2019/2022

### Key Components

- `MyNotePad.cpp` - Main program and window procedures
- `Dialog.cpp` - Dialog boxes and editing functionality
- `Document.cpp` - File I/O and encoding handling
- `Encoding.cpp` - Encoding detection and conversion
- `Utils.cpp` - Utility functions

### Dependencies

- Windows SDK
- ComCtl32.lib (Common Controls)
- comdlg32.lib (Common Dialogs)

## 🤝 Contributing

Issues and Pull Requests are welcome!

### Development Workflow

1. Fork the project
2. Create a feature branch (`git checkout -b feature/amazing-feature`)
3. Commit your changes (`git commit -m 'Add amazing feature'`)
4. Push the branch (`git push origin feature/amazing-feature`)
5. Create a Pull Request

### Code Standards

- Use TCHAR and related macros for Unicode support
- Follow Windows API naming conventions
- Add appropriate error handling and memory management

## 📄 License

This project is licensed under the MIT License - see the [LICENSE](LICENSE) file for details.

## Acknowledgments

This project recreates the functionality of Microsoft Windows Notepad as a programming exercise and educational tool, demonstrating classic Windows application development techniques.
