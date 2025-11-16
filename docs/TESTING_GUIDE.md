# Testing Guide for MyNotePad

This guide provides comprehensive information about testing in the MyNotePad project.

## Table of Contents

- [Overview](#overview)
- [Test Framework](#test-framework)
- [Running Tests](#running-tests)
- [Writing Tests](#writing-tests)
- [Test Coverage](#test-coverage)
- [Continuous Integration](#continuous-integration)
- [Troubleshooting](#troubleshooting)

## Overview

MyNotePad uses **Google Test** (gtest) framework for unit testing. The test suite is designed to verify:

- Core functionality (encoding, file I/O)
- Utility functions
- Edge cases and error handling
- Regression prevention

## Test Framework

### Google Test

**Why Google Test?**
- Industry standard for C++ testing
- Rich assertion library
- Test fixtures and parameterized tests
- Excellent IDE integration
- Cross-platform support

**Version**: Latest stable from vcpkg

**Documentation**: https://google.github.io/googletest/

## Running Tests

### Prerequisites

1. **Install vcpkg** (if not already installed):
   ```powershell
   git clone https://github.com/microsoft/vcpkg.git C:\vcpkg
   C:\vcpkg\bootstrap-vcpkg.bat
   ```

2. **Install Google Test**:
   ```powershell
   C:\vcpkg\vcpkg.exe install gtest:x86-windows
   ```

3. **Set environment variable**:
   ```powershell
   $env:VcpkgRoot = "C:\vcpkg\"
   ```

### Building Tests

#### Using Visual Studio

1. Open `MyNotePad.sln`
2. Right-click on `MyNotePad.Tests` project
3. Select "Build"

#### Using Command Line

```powershell
# Build Debug configuration
msbuild MyNotePad.Tests\MyNotePad.Tests.vcxproj /p:Configuration=Debug /p:Platform=Win32

# Build Release configuration
msbuild MyNotePad.Tests\MyNotePad.Tests.vcxproj /p:Configuration=Release /p:Platform=Win32
```

### Running Tests

#### Command Line - Basic

```powershell
# Run all tests
.\Debug\MyNotePad.Tests.exe

# Expected output:
# [==========] Running X tests from Y test suites.
# [----------] Global test environment set-up.
# ...
# [  PASSED  ] X tests.
```

#### Command Line - Advanced Options

```powershell
# Run with verbose output
.\Debug\MyNotePad.Tests.exe --gtest_verbose

# Run specific test suite
.\Debug\MyNotePad.Tests.exe --gtest_filter=EncodingTest.*

# Run specific test case
.\Debug\MyNotePad.Tests.exe --gtest_filter=EncodingTest.DetectUTF8BOM

# Run tests matching pattern
.\Debug\MyNotePad.Tests.exe --gtest_filter=*UTF8*

# Exclude tests
.\Debug\MyNotePad.Tests.exe --gtest_filter=-EncodingTest.*

# Repeat tests (useful for flaky test detection)
.\Debug\MyNotePad.Tests.exe --gtest_repeat=10

# Shuffle test order
.\Debug\MyNotePad.Tests.exe --gtest_shuffle

# Generate XML report
.\Debug\MyNotePad.Tests.exe --gtest_output=xml:test-results.xml

# Generate JSON report
.\Debug\MyNotePad.Tests.exe --gtest_output=json:test-results.json

# List all tests without running
.\Debug\MyNotePad.Tests.exe --gtest_list_tests
```

#### Visual Studio Test Explorer

1. Open **Test > Test Explorer**
2. Build the solution
3. Tests will appear in Test Explorer
4. Click "Run All" or run individual tests
5. View results inline with source code

#### Visual Studio Code

With C++ Test Adapter extension:

1. Install "C++ TestMate" extension
2. Configure `settings.json`:
   ```json
   {
     "testMate.cpp.test.executables": "Debug/MyNotePad.Tests.exe"
   }
   ```
3. Tests appear in Test Explorer sidebar
4. Run/debug tests directly from editor

## Writing Tests

### Test File Structure

```cpp
#include "gtest/gtest.h"
#include <windows.h>
#include "YourHeader.h"

// Test fixture (optional, for shared setup/teardown)
class ComponentTest : public ::testing::Test {
protected:
    // Runs before each test
    void SetUp() override {
        // Initialize resources
    }

    // Runs after each test
    void TearDown() override {
        // Cleanup resources
    }

    // Shared test data
    int testValue;
};

// Test case using fixture
TEST_F(ComponentTest, TestName) {
    // Arrange
    int expected = 42;

    // Act
    int actual = FunctionUnderTest();

    // Assert
    EXPECT_EQ(expected, actual);
}

// Simple test without fixture
TEST(ComponentTest, SimpleTest) {
    EXPECT_TRUE(SomeCondition());
}
```

### Common Assertions

#### Boolean Conditions
```cpp
EXPECT_TRUE(condition);
EXPECT_FALSE(condition);
```

#### Equality
```cpp
EXPECT_EQ(expected, actual);  // expected == actual
EXPECT_NE(val1, val2);        // val1 != val2
```

#### Comparison
```cpp
EXPECT_LT(val1, val2);  // val1 < val2
EXPECT_LE(val1, val2);  // val1 <= val2
EXPECT_GT(val1, val2);  // val1 > val2
EXPECT_GE(val1, val2);  // val1 >= val2
```

#### String Comparison
```cpp
EXPECT_STREQ(str1, str2);     // C-strings equal
EXPECT_STRNE(str1, str2);     // C-strings not equal
EXPECT_STRCASEEQ(str1, str2); // Case-insensitive equal
```

#### Floating Point
```cpp
EXPECT_FLOAT_EQ(val1, val2);  // Floats equal (with tolerance)
EXPECT_DOUBLE_EQ(val1, val2); // Doubles equal (with tolerance)
EXPECT_NEAR(val1, val2, abs_error); // Near with custom tolerance
```

#### Exceptions
```cpp
EXPECT_THROW(statement, exception_type);
EXPECT_NO_THROW(statement);
EXPECT_ANY_THROW(statement);
```

#### Death Tests (for crash testing)
```cpp
EXPECT_DEATH(statement, regex_pattern);
ASSERT_DEATH(statement, regex_pattern);
```

### EXPECT vs ASSERT

- **EXPECT_***: Continues test execution after failure
- **ASSERT_***: Stops test execution after failure

```cpp
TEST(MyTest, Example) {
    EXPECT_TRUE(condition1);  // Test continues even if fails
    ASSERT_TRUE(condition2);  // Test stops if this fails
    EXPECT_EQ(a, b);         // This won't run if ASSERT above failed
}
```

### Parameterized Tests

Test same logic with different inputs:

```cpp
// Define parameter type
class EncodingTestWithParam : public ::testing::TestWithParam<std::tuple<const char*, EncodingType>> {
};

// Test using parameters
TEST_P(EncodingTestWithParam, DetectEncoding) {
    auto [data, expectedEncoding] = GetParam();
    EncodingType detected = DetectEncoding(data);
    EXPECT_EQ(expectedEncoding, detected);
}

// Instantiate with different values
INSTANTIATE_TEST_SUITE_P(
    EncodingTests,
    EncodingTestWithParam,
    ::testing::Values(
        std::make_tuple("\xEF\xBB\xBF", ENCODING_UTF8_BOM),
        std::make_tuple("\xFF\xFE", ENCODING_UTF16_LE),
        std::make_tuple("\xFE\xFF", ENCODING_UTF16_BE)
    )
);
```

### Test Fixtures for Windows Resources

```cpp
class WindowsResourceTest : public ::testing::Test {
protected:
    void SetUp() override {
        // Allocate Windows resources
        hMem = LocalAlloc(LMEM_ZEROINIT, 1024);
        ASSERT_NE(hMem, nullptr) << "Failed to allocate memory";
    }

    void TearDown() override {
        // Always cleanup, even if test fails
        if (hMem) {
            LocalFree(hMem);
            hMem = nullptr;
        }
    }

    HLOCAL hMem;
};

TEST_F(WindowsResourceTest, UseMemory) {
    // hMem is available and will be cleaned up automatically
    EXPECT_NE(hMem, nullptr);
}
```

## Test Coverage

### Current Coverage

As of the current version, tests cover:

- **Encoding Detection**: Placeholder tests for UTF-8, UTF-16, GBK
- **Line Ending Normalization**: Placeholder tests
- **Utility Functions**: Basic memory allocation test

### Expanding Coverage

Priority areas for test expansion:

1. **High Priority**:
   - Actual encoding detection implementation
   - File I/O operations
   - Memory management functions

2. **Medium Priority**:
   - Text manipulation functions
   - Configuration persistence (Registry)
   - Error handling paths

3. **Low Priority**:
   - UI-related code (harder to test, consider integration tests)
   - Print formatting
   - Dialog interactions

### Coverage Tools

To measure code coverage:

1. **OpenCppCoverage** (Windows):
   ```powershell
   # Install
   choco install opencppcoverage

   # Run with coverage
   OpenCppCoverage.exe --sources MyNotePad -- Debug\MyNotePad.Tests.exe

   # Generate HTML report
   OpenCppCoverage.exe --sources MyNotePad --export_type html:coverage -- Debug\MyNotePad.Tests.exe
   ```

2. **Visual Studio** (Enterprise):
   - Test > Analyze Code Coverage for All Tests

## Continuous Integration

### GitHub Actions

Tests run automatically on:
- Every push to main/master
- Every pull request
- Release creation

### CI Test Execution

```yaml
- name: Run Unit Tests
  run: |
    .\Debug\MyNotePad.Tests.exe --gtest_output=xml:test-results.xml
```

### Viewing CI Results

1. Go to repository → Actions tab
2. Select workflow run
3. View "Unit Tests" job
4. Download test-results artifact for detailed XML report

### Failing Tests in CI

If tests fail in CI:

1. Check the job logs for failure details
2. Download test results artifact
3. Reproduce locally with same configuration
4. Fix the issue
5. Push the fix

## Troubleshooting

### Tests Don't Build

**Issue**: Cannot find gtest/gtest.h

**Solution**:
```powershell
# Ensure vcpkg is installed
C:\vcpkg\vcpkg.exe install gtest:x86-windows

# Verify VcpkgRoot environment variable
echo $env:VcpkgRoot

# Integrate with Visual Studio (one time)
C:\vcpkg\vcpkg.exe integrate install
```

### Linker Errors

**Issue**: LNK2019 unresolved external symbol

**Solutions**:
1. Check runtime library settings match:
   - Debug: Multi-threaded Debug DLL (MDd)
   - Release: Multi-threaded DLL (MD)

2. Verify gtest libraries are linked:
   - `gtest.lib`
   - `gtest_main.lib`

3. Rebuild Google Test if needed:
   ```powershell
   C:\vcpkg\vcpkg.exe remove gtest:x86-windows
   C:\vcpkg\vcpkg.exe install gtest:x86-windows
   ```

### Tests Crash

**Issue**: Access violation or crash during test execution

**Debug Steps**:
1. Run under debugger (F5 in Visual Studio)
2. Enable all exceptions: Debug > Windows > Exception Settings > Check all
3. Check for:
   - Null pointer dereferences
   - Uninitialized variables
   - Buffer overruns
   - Invalid handles

### Flaky Tests

**Issue**: Tests pass sometimes, fail other times

**Investigation**:
1. Run test repeatedly:
   ```powershell
   .\Debug\MyNotePad.Tests.exe --gtest_repeat=100 --gtest_filter=FlakyTest
   ```

2. Common causes:
   - Uninitialized variables
   - Race conditions (if multi-threaded)
   - Timing dependencies
   - Environmental dependencies (files, registry)

3. Fix by:
   - Proper initialization in SetUp()
   - Removing timing assumptions
   - Mocking external dependencies

### Test Results Not Uploaded in CI

**Issue**: Artifacts missing in GitHub Actions

**Check**:
1. Test executable path is correct
2. Test runs to completion (doesn't crash)
3. XML output file is generated
4. Artifact upload step has correct path

## Best Practices

### DO

✅ Write tests for new features before implementation (TDD)
✅ Keep tests small and focused (one concept per test)
✅ Use descriptive test names (TestMethodName_StateUnderTest_ExpectedBehavior)
✅ Use fixtures for common setup/teardown
✅ Clean up resources in TearDown()
✅ Test edge cases and error paths
✅ Make tests deterministic (same input = same output)
✅ Run tests before committing code

### DON'T

❌ Don't test private implementation details
❌ Don't write tests that depend on execution order
❌ Don't use global state or static variables
❌ Don't skip test failures (fix them!)
❌ Don't make tests too complex (if test is complex, simplify production code)
❌ Don't test the framework itself
❌ Don't ignore flaky tests

## Resources

- [Google Test Primer](https://google.github.io/googletest/primer.html)
- [Google Test Advanced Guide](https://google.github.io/googletest/advanced.html)
- [Google Test FAQ](https://google.github.io/googletest/faq.html)
- [Google Test Samples](https://github.com/google/googletest/tree/main/googletest/samples)

## Contributing Tests

When contributing tests:

1. Follow existing test structure and naming
2. Add tests in appropriate test file (or create new one)
3. Ensure tests pass locally before submitting PR
4. Update this documentation if adding new test patterns
5. Aim for meaningful assertions, not just coverage percentage

---

For CI/CD specific information, see [CI_CD_ENHANCED.md](CI_CD_ENHANCED.md).
