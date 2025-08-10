# 文件系统演示模块

## 概述

文件系统模块展示了 Losu 语言中的文件操作功能，基于 Emscripten 的文件系统 API 实现，提供了完整的文件和目录操作能力。

## 功能特性

### 📁 基础文件操作
- **读取文件** - `fs.read(path)`: 读取指定路径的文件内容
- **写入文件** - `fs.write(path, content)`: 写入内容到指定文件
- **追加内容** - `fs.append(path, content)`: 向文件末尾追加内容
- **删除文件** - `fs.remove(path)`: 删除指定文件
- **重命名文件** - `fs.rename(old_path, new_path)`: 重命名或移动文件

### 📂 目录操作
- **创建目录** - 创建新的目录结构
- **列出目录** - 查看目录中的文件和子目录
- **目录遍历** - 递归访问目录树

### 📊 文件信息
- **文件统计** - 获取文件大小、修改时间等元数据
- **类型检查** - 判断是文件还是目录
- **权限信息** - 查看文件访问权限

## API 参考

### fs.read(path)
读取文件内容并返回字符串。

```losu
let content = fs.read("/demo/hello.txt")
print("文件内容: " + content)
```

### fs.write(path, content)
写入内容到文件，如果文件不存在则创建。

```losu
fs.write("/demo/output.txt", "Hello, World!")
```

### fs.append(path, content)
向文件末尾追加内容。

```losu
fs.append("/demo/log.txt", "新的日志条目\n")
```

### fs.rename(old_path, new_path)
重命名或移动文件。

```losu
fs.rename("/demo/old_name.txt", "/demo/new_name.txt")
```

### fs.remove(path)
删除指定文件。

```losu
fs.remove("/demo/temp_file.txt")
```

## 技术实现

### WebAssembly 集成
- 使用 Emscripten 编译 C 代码到 WebAssembly
- 利用 Emscripten 的文件系统 API (FS)
- 支持内存文件系统 (MEMFS) 操作

### 文件系统架构
```
Browser Memory FileSystem (MEMFS)
├── /demo/                  # 演示目录
│   ├── hello.txt          # 示例文件
│   ├── data.txt           # 数据文件
│   └── subdir/            # 子目录
└── /tmp/                  # 临时目录
```

### 错误处理
- 文件不存在异常
- 权限错误处理
- 路径格式验证
- 内存分配失败处理

## 使用示例

### 基本文件操作
```losu
import fs

// 创建并写入文件
fs.write("/demo/test.txt", "测试内容")

// 读取文件
let content = fs.read("/demo/test.txt")
print("读取到: " + content)

// 追加内容
fs.append("/demo/test.txt", "\n追加的内容")

// 重命名文件
fs.rename("/demo/test.txt", "/demo/renamed.txt")

// 删除文件
fs.remove("/demo/renamed.txt")
```

### 文件管理流程
```losu
import fs

// 1. 创建配置文件
let config = '{"version": "1.0", "name": "MyApp"}'
fs.write("/demo/config.json", config)

// 2. 创建日志文件
fs.write("/demo/app.log", "应用启动\n")

// 3. 追加日志
fs.append("/demo/app.log", "用户登录\n")
fs.append("/demo/app.log", "数据加载完成\n")

// 4. 读取配置
let app_config = fs.read("/demo/config.json")
print("应用配置: " + app_config)

// 5. 备份重要文件
let log_content = fs.read("/demo/app.log")
fs.write("/demo/app.backup.log", log_content)
```

## 最佳实践

### 路径规范
- 使用绝对路径 `/demo/file.txt`
- 避免特殊字符和空格
- 使用正斜杠 `/` 作为路径分隔符

### 错误处理
```losu
import fs

try:
    let content = fs.read("/demo/nonexistent.txt")
    print(content)
except error:
    print("文件读取失败: " + error)
```

### 性能优化
- 避免频繁的小文件写入
- 批量处理文件操作
- 合理使用文件缓存

## 演示功能

### 交互式操作
- 📖 **读取文件** - 选择文件路径进行读取
- ✏️ **写入文件** - 输入内容创建新文件  
- 📊 **文件信息** - 查看文件的详细元数据
- 🗑️ **删除文件** - 安全删除指定文件
- 📂 **列出目录** - 浏览目录内容
- 📁 **创建目录** - 建立新的目录结构
- 🔄 **重命名** - 重命名文件或目录

### 代码示例
提供三个完整的演示脚本：
1. **基础文件操作** - 文件读写、追加、验证
2. **目录操作** - 文件管理、重命名、清理
3. **文件管理** - 完整的项目文件管理流程

## 注意事项

### 浏览器限制
- 文件系统是内存虚拟的，刷新页面后数据会丢失
- 无法访问用户本地文件系统
- 文件大小受浏览器内存限制

### 安全考虑
- 所有操作在沙箱环境中进行
- 不能访问系统敏感文件
- 路径访问受到严格限制

### 兼容性
- 支持现代浏览器
- 需要 WebAssembly 支持
- 建议使用 Chrome/Firefox/Safari 最新版本