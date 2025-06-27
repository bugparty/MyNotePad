# MyNotePad

A modern Windows Notepad application with enhanced editing features and print support.

![Build Status](https://github.com/bugparty/MyNotePad/workflows/Build%20and%20Release%20MyNotePad/badge.svg)

![My Notepad Screenshot](./Screenshot01.png)


## ✨ Features

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

## 🏗️ Build Instructions

### Local Build

#### System Requirements

- Windows 10/11
- Visual Studio 2019/2022 (Community edition is sufficient)
- Windows SDK

#### Build Steps

```bash
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

```bash
# Create version tag
git tag v1.0.0
git push origin v1.0.0

# Create Release on GitHub to trigger automatic build and file attachment
```

## 📦 Downloads

### Download from Releases

Visit the [Releases page](https://github.com/bugparty/MyNotePad/releases) to download the latest version.

### Download from Artifacts

After each build, you can download the latest build results from the Actions page.

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

## 📝 Version History

- **v1.0.0** - Initial release
  - Basic text editing functionality
  - Find and replace features
  - File operations and encoding support
  - Print functionality

## 📄 License

This project is licensed under the MIT License - see the [LICENSE](LICENSE) file for details.

## 🙏 Acknowledgments

- Microsoft - Windows API and development tools
- GitHub - Code hosting and Actions service
- Open source community - Various references and inspiration

- **Text Editing**: Full text editing capabilities with support for common operations
- **File Operations**:
  - Open existing text files
  - Save and Save As functionality
  - Create new documents
- **Edit Menu**: Standard editing operations including:
  - Undo
  - Copy, Cut, Paste
  - Find text with options for case matching and whole word search
  - Replace text with single and batch replace functionality
- **Scrolling**: Vertical scrolling support for large documents
- **Native Windows Integration**: Built using pure Win32 API for authentic Windows experience

## Usage

Run the compiled `MyNotePad.exe` to start the application. The interface closely mimics the classic Windows Notepad:

- **File Menu**: New, Open, Save, Save As, Print, Exit
- **Edit Menu**: Undo, Cut, Copy, Paste
- **Help Menu**: About dialog

## Implementation Highlights

- **Win32 Window Management**: Custom window class registration and message handling
- **Edit Control**: Uses Windows' built-in edit control for text editing functionality
- **Scrolling**: Implements custom vertical scrolling with scroll bar management
- **File I/O**: Native Windows file operations for opening and saving documents
- **Resource Management**: Proper handling of Windows resources (DC, handles, etc.)

## Code Features

- Message-driven architecture using `WndProc` callback
- Proper resource cleanup and memory management
- Unicode support through TCHAR usage
- Common Controls integration for enhanced UI elements

## Contributing

This project serves as an educational example of Win32 programming. Feel free to:

- Report issues or bugs
- Suggest improvements
- Submit pull requests
- Use as a reference for learning Win32 API programming

## License

This project is open source. Feel free to use, modify, and distribute according to your needs.

## Acknowledgments

This project recreates the functionality of Microsoft Windows Notepad as a programming exercise and educational tool, demonstrating classic Windows application development techniques.