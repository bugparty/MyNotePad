# MyNotePad Unit Tests

This directory contains unit tests for the MyNotePad application using Google Test framework.

## Prerequisites

To build and run the tests, you need:

1. **Visual Studio 2022** (or 2019) with C++ development tools
2. **vcpkg** package manager
3. **Google Test** library

## Setup

### Installing Google Test via vcpkg

```powershell
# Install vcpkg if not already installed
git clone https://github.com/microsoft/vcpkg.git C:\vcpkg
C:\vcpkg\bootstrap-vcpkg.bat

# Install Google Test for x86-windows
C:\vcpkg\vcpkg.exe install gtest:x86-windows

# Set environment variable (if not already set)
$env:VcpkgRoot = "C:\vcpkg\"
```

### Integrating with Visual Studio

```powershell
# Integrate vcpkg with Visual Studio (optional, but recommended)
C:\vcpkg\vcpkg.exe integrate install
```

## Building Tests

### Using Visual Studio

1. Open `MyNotePad.sln` in Visual Studio
2. Add the test project to the solution (if not already added)
3. Set build configuration to Debug or Release
4. Build the `MyNotePad.Tests` project

### Using Command Line

```powershell
# Build test project
msbuild MyNotePad.Tests\MyNotePad.Tests.vcxproj /p:Configuration=Debug /p:Platform=Win32
```

## Running Tests

### From Command Line

```powershell
# Run all tests
.\Debug\MyNotePad.Tests.exe

# Run with verbose output
.\Debug\MyNotePad.Tests.exe --gtest_verbose

# Run specific test
.\Debug\MyNotePad.Tests.exe --gtest_filter=EncodingTest.DetectUTF8BOM

# Generate XML report
.\Debug\MyNotePad.Tests.exe --gtest_output=xml:test-results.xml
```

### From Visual Studio Test Explorer

1. Open Test Explorer (Test > Test Explorer)
2. Click "Run All" to run all tests
3. View results in the Test Explorer window

## Test Structure

```
MyNotePad.Tests/
├── EncodingTests.cpp      # Tests for encoding detection and conversion
├── UtilsTests.cpp         # Tests for utility functions
├── MyNotePad.Tests.vcxproj # Visual Studio project file
└── README.md              # This file
```

## Writing New Tests

### Test File Template

```cpp
#include "gtest/gtest.h"
#include <windows.h>

// Test fixture
class MyComponentTest : public ::testing::Test {
protected:
    void SetUp() override {
        // Setup code before each test
    }

    void TearDown() override {
        // Cleanup code after each test
    }
};

// Test case
TEST_F(MyComponentTest, TestName) {
    // Arrange
    int expected = 42;

    // Act
    int actual = SomeFunction();

    // Assert
    EXPECT_EQ(expected, actual);
}
```

### Google Test Assertions

Common assertions:
- `EXPECT_TRUE(condition)` - Expects condition to be true
- `EXPECT_FALSE(condition)` - Expects condition to be false
- `EXPECT_EQ(expected, actual)` - Expects values to be equal
- `EXPECT_NE(val1, val2)` - Expects values to be not equal
- `EXPECT_LT(val1, val2)` - Expects val1 < val2
- `EXPECT_GT(val1, val2)` - Expects val1 > val2
- `EXPECT_STREQ(str1, str2)` - Expects C strings to be equal

Use `ASSERT_*` instead of `EXPECT_*` to stop test execution on failure.

## Current Test Coverage

### Implemented Tests

Currently, the tests are in **placeholder mode**. The following test cases are defined but need actual implementation:

#### Encoding Tests
- ✅ UTF-8 BOM detection (placeholder)
- ✅ UTF-16 LE BOM detection (placeholder)
- ✅ UTF-16 BE BOM detection (placeholder)
- ✅ Valid UTF-8 sequence validation (placeholder)
- ✅ Invalid UTF-8 sequence validation (placeholder)
- ✅ GBK detection (placeholder)
- ✅ Line ending normalization (LF, CR, CRLF) (placeholder)

#### Utility Tests
- ✅ Error message formatting (placeholder)
- ✅ Memory allocation (basic test implemented)

### Next Steps

To make tests functional:

1. **Refactor code for testability**:
   - Extract encoding functions to a separate testable module
   - Create interfaces or wrapper functions that can be tested
   - Avoid tight coupling with Windows message handlers

2. **Implement test functions**:
   - Replace `EXPECT_TRUE(true)` placeholders with actual tests
   - Link test code with actual implementation
   - Add more comprehensive test cases

3. **Add more test coverage**:
   - File I/O operations
   - Text editing operations
   - Dialog functionality
   - Print formatting

## Continuous Integration

Tests are automatically run in GitHub Actions CI pipeline:

```yaml
- name: Run Unit Tests
  run: .\Debug\MyNotePad.Tests.exe --gtest_output=xml:test-results.xml
```

## Troubleshooting

### Common Issues

**Issue**: `fatal error: gtest/gtest.h: No such file or directory`

**Solution**: Ensure vcpkg is installed and Google Test is installed for the correct platform:
```powershell
C:\vcpkg\vcpkg.exe install gtest:x86-windows
```

**Issue**: `LNK2019: unresolved external symbol` errors

**Solution**: Make sure:
1. Google Test libraries are in the library path
2. Runtime library settings match (MD/MT/MDd/MTd)
3. vcpkg integration is enabled

**Issue**: Tests fail to run with access violation

**Solution**: Check that:
1. All pointers are properly initialized
2. Memory is allocated before use
3. Windows handles are valid before use

## References

- [Google Test Documentation](https://google.github.io/googletest/)
- [Google Test Primer](https://google.github.io/googletest/primer.html)
- [vcpkg Documentation](https://vcpkg.io/)
