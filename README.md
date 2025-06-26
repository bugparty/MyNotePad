# MyNotePad

A high-fidelity clone of the classic Windows Notepad application, built using pure Win32 API in C++.

## Overview

This project is a faithful recreation of the traditional Windows Notepad text editor, implementing the core functionality and user interface using native Windows API calls. The application demonstrates classic Win32 programming techniques and provides a lightweight, efficient text editing experience similar to the original Notepad.

## Features

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

## How to Use Find and Replace

### Find Dialog
1. Go to **Edit > Find** or press **Ctrl+F** (if implemented)
2. Enter the text you want to find in the "Find what:" field
3. Optionally check:
   - **Match case**: to perform case-sensitive search
   - **Whole word**: to match complete words only
4. Click **Find Next** or **Find Previous** to navigate through matches
5. Click **Close** to close the dialog

### Replace Dialog
1. Go to **Edit > Replace** or press **Ctrl+H** (if implemented)
2. Enter the text you want to find in the "Find what:" field
3. Enter the replacement text in the "Replace with:" field
4. Optionally check:
   - **Match case**: to perform case-sensitive search
   - **Whole word**: to match complete words only
5. Use the following buttons:
   - **Find Next**: locate the next occurrence without replacing
   - **Replace**: replace the current selection and find next
   - **Replace All**: replace all occurrences in the document
   - **Close**: close the dialog

### Features
- Both dialogs are **modeless**, allowing you to continue editing while they remain open
- Search text and options are preserved between sessions
- Find and Replace dialogs share the same search settings

## Technical Implementation

- **Language**: C++
- **API**: Win32 API
- **IDE**: Visual Studio (compatible with VS 2019/2022)
- **Target Platform**: Windows

## Project Structure

```text
MyNotePad/
├── MyNotePad.sln          # Visual Studio solution file
├── MyNotePad/
│   ├── MyNotePad.cpp      # Main application entry point and window procedures
│   ├── MyNotePad.h        # Main header file
│   ├── Dialog.cpp/.h      # Dialog handling (File Open/Save)
│   ├── Document.cpp/.h    # Document management
│   ├── Utils.cpp/.h       # Utility functions
│   ├── Resource.h         # Resource definitions
│   ├── MyNotePad.rc       # Resource file
│   └── res/              # Application icons and resources
└── Debug/                # Build output directory
```

## Building the Project

### Prerequisites

- Visual Studio 2019 or later
- Windows SDK
- C++ development tools

### Build Instructions

1. Clone or download this repository
2. Open `MyNotePad.sln` in Visual Studio
3. Build the solution (Ctrl+Shift+B)
4. The executable will be generated in the `Debug/` folder

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