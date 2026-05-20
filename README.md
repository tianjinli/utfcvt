# utfcvt - Multi-byte to Unicode Converter

**[中文](README_CN.md) | English**

---

## English README

### Overview

**utfcvt** is a Windows desktop application and command-line tool for converting text files between various character encodings, with a focus on converting MBCS (Multi-Byte Character Sets) to Unicode/UTF-8. It provides an intuitive GUI with full multi-language support (English and Chinese).

### Key Features

- **Smart Detection**: Automatically detects file encoding with fallback strategies
- **GUI & CLI**: Both graphical and command-line interfaces available
- **Multi-language UI**: Interface supports English and Chinese (Simplified)
- **Async Processing**: Leverages C++ coroutines for responsive UI during bulk operations
- **Filtering**: Exclude specific folders and filter by file extensions

### Requirements

- **Compiler**: Visual Studio 2022 or later (C++23 required for coroutines)
- **Platform**: Windows (uses Win32 API)
- **Character Set**: Unicode/UTF-8 project configuration

### Usage

#### GUI Mode

1. Launch `utfcvt.exe`
2. Select source and target folders
3. Configure file extensions and folders to exclude
4. Choose source and target encodings
5. Click "Start conversion" to begin

#### Command-Line Mode

```bash
utfcvt --help
```

Options include source/target encodings, file extensions, folders to exclude, and more.

### Building

1. Open `utfcvt.sln` in Visual Studio
2. Select the desired configuration (Debug/Release)
3. Build the solution

### Testing

Test files are provided in the `Data/` directory:
- `ANSI.txt` - ANSI/GBK encoded file
- `UTF8.txt` - UTF-8 encoded file
- `UTF8BOM.txt` - UTF-8 with BOM
- `UTF16LE.txt` - UTF-16 LE encoded file

---
