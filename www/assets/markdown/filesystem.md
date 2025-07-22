# 基于洛书（Losu）编程语言与 Web Assembly 的编译与 OS 演示系统

---

## 项目简介

本项目基于洛书（Losu）编程语言与 Web Assembly 技术，打造一个集编程教学与操作系统原理演示于一体的交互式网站。用户可以在浏览器中编写 Losu 代码，体验编译、虚拟机运行、内存管理、线程调度、文件操作等底层 OS 功能的可视化过程，助力系统能力学习与创新实践。

本项目为 2025 年全国大学生计算机系统能力大赛-操作系统设计赛（全国）- OS功能挑战赛道参赛作品。

## 文件系统实验介绍

### 实验概述

文件系统是操作系统的重要组成部分，负责数据的持久化存储和管理。本实验基于 WebAssembly 的虚拟文件系统，展示洛书语言如何通过系统调用与文件系统交互，帮助学生理解文件系统的基本原理和操作方法。

### 学习目标

- **理解文件系统的作用**：掌握文件系统在操作系统中的重要地位
- **掌握文件操作原理**：学习文件的创建、读写、删除等基本操作
- **了解虚拟文件系统**：理解 WebAssembly 环境下的文件系统实现
- **体验系统调用过程**：通过洛书语言体验文件相关的系统调用

### 实验内容

#### 1. WebAssembly 文件系统架构

**WASM FS 特点**
- **内存文件系统**：文件存储在浏览器内存中，提供高速访问
- **虚拟目录结构**：模拟传统文件系统的目录层次结构
- **标准接口支持**：兼容 POSIX 文件操作接口
- **持久化选项**：支持与浏览器 LocalStorage 或 IndexedDB 集成

**文件系统层次**
```
Virtual File System (VFS)
├── /tmp/           # 临时文件目录
├── /home/          # 用户文件目录
├── /usr/           # 系统文件目录
│   ├── bin/        # 可执行文件
│   └── lib/        # 库文件
└── /dev/           # 设备文件
    ├── stdin       # 标准输入
    ├── stdout      # 标准输出
    └── stderr      # 标准错误
```

#### 2. 洛书文件系统扩展

**fs 模块功能**
洛书语言内置的文件系统模块提供以下功能：

```losu
import fs

# 文件读取
content = fs::read("path/to/file.txt")

# 文件写入
fs::write("output.txt", "Hello, World!")

# 文件追加
fs::append("log.txt", "New log entry\n")

# 文件重命名
fs::rename("old_name.txt", "new_name.txt")

# 文件删除
fs::remove("temp_file.txt")
```

**系统调用映射**
| 洛书函数 | 系统调用 | 功能描述 |
|---------|---------|---------|
| `fs::read()` | `open()`, `read()`, `close()` | 读取文件内容 |
| `fs::write()` | `open()`, `write()`, `close()` | 写入文件内容 |
| `fs::append()` | `open()`, `write()`, `close()` | 追加文件内容 |
| `fs::rename()` | `rename()` | 重命名文件 |
| `fs::remove()` | `unlink()` | 删除文件 |

#### 3. 文件操作实验场景

**基本文件操作**
```losu
import fs

# 创建和写入文件
fs::write("demo.txt", "这是一个演示文件\n")
print("文件创建成功")

# 读取文件内容
let content = fs::read("demo.txt")
print("文件内容：", content)

# 追加内容
fs::append("demo.txt", "追加的新内容\n")

# 再次读取
let updated_content = fs::read("demo.txt")
print("更新后的内容：", updated_content)
```

**文件管理操作**
```losu
import fs

# 创建多个文件
let files = ["file1.txt", "file2.txt", "file3.txt"]
for filename in files:
    fs::write(filename, "内容: " & filename)

# 重命名文件
fs::rename("file1.txt", "renamed_file.txt")

# 删除文件
fs::remove("file2.txt")

print("文件操作完成")
```

**数据处理示例**
```losu
import fs

# 处理CSV数据
def process_csv(filename):
    let content = fs::read(filename)
    let lines = content.split("\n")
    let result = []
    
    for line in lines:
        if line.length() > 0:
            let fields = line.split(",")
            result.append(fields)
    
    return result

# 创建示例CSV文件
let csv_data = "姓名,年龄,城市\n张三,25,北京\n李四,30,上海\n王五,28,广州"
fs::write("data.csv", csv_data)

# 处理CSV数据
let parsed_data = process_csv("data.csv")
print("解析的数据：", parsed_data)
```

**日志系统示例**
```losu
import fs
import time

class Logger:
    def __init__(self, logfile):
        self.logfile = logfile
    
    def log(self, level, message):
        let timestamp = time::now()
        let log_entry = f"[{timestamp}] {level}: {message}\n"
        fs::append(self.logfile, log_entry)
    
    def info(self, message):
        self.log("INFO", message)
    
    def error(self, message):
        self.log("ERROR", message)
    
    def warning(self, message):
        self.log("WARNING", message)

# 使用日志系统
let logger = Logger("app.log")
logger.info("应用启动")
logger.warning("这是一个警告")
logger.error("发生了错误")

# 读取日志文件
let log_content = fs::read("app.log")
print("日志内容：\n", log_content)
```

#### 4. 文件系统性能测试

**读写性能测试**
```losu
import fs
import time

def benchmark_write(filename, size_mb):
    let data = "x" * (size_mb * 1024 * 1024)  # 创建指定大小的数据
    
    let start_time = time::now()
    fs::write(filename, data)
    let end_time = time::now()
    
    let duration = end_time - start_time
    print(f"写入 {size_mb}MB 数据耗时: {duration} 秒")
    
    return duration

def benchmark_read(filename):
    let start_time = time::now()
    let content = fs::read(filename)
    let end_time = time::now()
    
    let duration = end_time - start_time
    let size_mb = content.length() / (1024 * 1024)
    print(f"读取 {size_mb}MB 数据耗时: {duration} 秒")
    
    return duration

# 执行性能测试
benchmark_write("test_1mb.txt", 1)
benchmark_read("test_1mb.txt")
```

**并发文件操作**
```losu
import fs

async def writer_task(task_id, count):
    for i in 1, count:
        let filename = f"task_{task_id}_file_{i}.txt"
        let content = f"Task {task_id}, File {i}\n"
        fs::write(filename, content)
        yield  # 让出控制权
    print(f"写入任务 {task_id} 完成")

async def reader_task(task_id, filenames):
    for filename in filenames:
        try:
            let content = fs::read(filename)
            print(f"读取任务 {task_id} 读取了 {filename}")
        except:
            print(f"文件 {filename} 不存在")
        yield

# 启动并发任务
async writer_task(1, 5)
async writer_task(2, 5)

let filenames = ["task_1_file_1.txt", "task_2_file_1.txt"]
async reader_task(1, filenames)
```

#### 5. 错误处理和异常管理

**文件不存在处理**
```losu
import fs

def safe_read(filename):
    try:
        return fs::read(filename)
    except FileNotFoundError:
        print(f"文件 {filename} 不存在")
        return null
    except PermissionError:
        print(f"没有权限访问文件 {filename}")
        return null
    except:
        print(f"读取文件 {filename} 时发生未知错误")
        return null

# 测试错误处理
let content = safe_read("nonexistent.txt")
if content != null:
    print("文件内容：", content)
else:
    print("无法读取文件")
```

### 操作步骤

1. **访问文件系统页面**：在主页侧边栏点击"文件系统"进入实验页面
2. **了解文件系统结构**：查看虚拟文件系统的目录结构
3. **选择实验场景**：从预设的文件操作示例中选择，或编写自定义代码
4. **执行文件操作**：运行洛书代码，观察文件系统的变化
5. **监控系统调用**：查看文件操作对应的系统调用：
   - 调用次数统计
   - 操作耗时分析
   - 错误情况记录
6. **文件系统状态查看**：实时查看文件系统的状态变化

### 实验重点

#### 文件操作理解
- **文件生命周期**：了解文件从创建到删除的完整过程
- **系统调用开销**：理解文件操作的性能成本
- **错误处理机制**：掌握文件操作中的异常处理

#### 虚拟文件系统
- **内存存储**：理解基于内存的文件系统特点
- **接口兼容性**：了解虚拟文件系统如何兼容标准接口
- **性能特征**：掌握虚拟文件系统的性能特点

#### 数据持久化
- **临时存储**：理解会话级别的数据存储
- **持久化选项**：了解如何实现数据的长期存储
- **数据同步**：掌握数据同步和备份的方法

### 技术实现

- **Emscripten FS API**：基于 Emscripten 的文件系统实现
- **POSIX兼容接口**：提供标准的文件操作接口
- **异步I/O支持**：支持非阻塞的文件操作
- **错误映射**：将系统错误映射到洛书异常

### 文件系统优化

**缓存机制**
- 文件内容缓存：避免重复读取
- 元数据缓存：加速文件属性查询
- 写入缓冲：批量写入提高性能

**内存管理**
- 自动垃圾回收：清理无用的文件缓存
- 内存限制：控制文件系统的内存使用
- 压缩存储：对大文件进行压缩存储

### 实验价值

通过文件系统实验，学生将：
- 深入理解文件系统的工作原理
- 掌握文件操作的编程技巧
- 了解虚拟文件系统的实现方法
- 学会处理文件操作中的各种异常情况

### 扩展思考

1. **文件系统类型比较**：内存文件系统 vs 磁盘文件系统的优劣
2. **并发控制**：多进程/多线程环境下的文件访问控制
3. **数据一致性**：如何保证文件操作的原子性和一致性
4. **性能优化**：大文件处理和高并发场景下的优化策略

通过本实验，学生将全面了解文件系统的设计原理和实现方法，为系统编程和应用开发奠定基础。 