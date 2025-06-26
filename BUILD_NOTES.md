# MyNotePad 构建和开发笔记

## MSBuild 编译命令

### 正确的MSBuild路径
用命令查找MSBuild
```powershell
Get-ChildItem "C:\Program Files*" -Recurse -Name "MSBuild.exe" -ErrorAction SilentlyContinue | Select-Object -First 3
```
目前机器的路径
```powershell
cd "c:\Users\fancy\sources\MyNotePad"
& "C:\Program Files\Microsoft Visual Studio\2022\Community\MSBuild\Current\Bin\MSBuild.exe" MyNotePad.sln /p:Configuration=Debug /p:Platform=Win32 /t:Rebuild
```

### 说明
- **路径**: `C:\Program Files\Microsoft Visual Studio\2022\Community\MSBuild\Current\Bin\MSBuild.exe`
- **平台**: Win32 (不是x86)
- **配置**: Debug
- **操作**: Rebuild (完全重新构建)

### 常见问题
1. **文件被占用错误**: 如果出现 "cannot open file" 错误，先关闭正在运行的程序：
   ```powershell
   taskkill /f /im MyNotePad.exe
   ```

2. **平台设置**: 确保使用 `/p:Platform=Win32` 而不是 `/p:Platform=x86`

## 已完成的功能

### ✅ 查找和替换功能
- [x] 查找对话框 (Find Dialog)
- [x] 替换对话框 (Replace Dialog)
- [x] 单个替换功能
- [x] 全部替换功能
- [x] 大小写匹配选项
- [x] 整词匹配选项
- [x] 非模态对话框实现

### ✅ 编辑控件修复
- [x] 修复回车键换行问题 (`ES_WANTRETURN` 样式)
- [x] 多行文本编辑支持
- [x] 滚动条支持

## 下次工作计划

### 🔄 可能需要的改进
1. **键盘快捷键**
   - [ ] Ctrl+F 快捷键打开查找对话框
   - [ ] Ctrl+H 快捷键打开替换对话框
   - [ ] F3 查找下一个
   - [ ] Shift+F3 查找上一个

2. **用户体验优化**
   - [ ] 添加状态栏显示查找/替换结果
   - [ ] 改进对话框位置记忆
   - [ ] 添加查找历史记录

3. **功能扩展**
   - [ ] 正则表达式查找
   - [ ] 多文件查找替换
   - [ ] 语法高亮

### 📝 技术债务
- [ ] 代码注释中文化
- [ ] 错误处理完善
- [ ] 内存泄漏检查

## 代码结构

### 关键文件
- `Dialog.h/cpp`: 对话框功能实现
- `MyNotePad.cpp`: 主程序和菜单处理
- `Resource.h`: 资源ID定义
- `MyNotePad.rc`: 界面资源定义

### 重要函数
- `CreateEditControl()`: 创建主编辑控件
- `ShowFindDialog()`: 显示查找对话框
- `ShowReplaceDialog()`: 显示替换对话框
- `FindTextInEdit()`: 查找文本核心逻辑
- `ReplaceTextInEdit()`: 替换文本核心逻辑

## 调试技巧

### 运行程序
```powershell
cd "c:\Users\fancy\sources\MyNotePad\Debug"
.\MyNotePad.exe
```

### 强制关闭程序
```powershell
taskkill /f /im MyNotePad.exe
```

### 技术实现细节

#### 回车键换行修复
问题的根源是Windows编辑控件默认情况下不处理回车键换行。解决方案包括：

1. **ES_WANTRETURN 样式**: 告诉编辑控件它应该接收回车键
2. **子类化处理**: 
   - `WM_GETDLGCODE`: 返回 `DLGC_WANTALLKEYS` 告诉系统我们要处理所有按键
   - `WM_CHAR`: 拦截回车键 (VK_RETURN) 并手动插入 `\r\n`

```cpp
// 子类化代码示例
case WM_GETDLGCODE:
    return DLGC_WANTALLKEYS;
case WM_CHAR:
    if (wParam == VK_RETURN) {
        SendMessage(hWnd, EM_REPLACESEL, TRUE, (LPARAM)_T("\r\n"));
        return 0;
    }
```
---
**最后更新**: 2025年6月25日
**状态**: 查找替换功能已完成，回车换行问题已修复，回车已经修复