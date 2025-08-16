# Losu 编译器演示模块使用指南

## 概述

本项目实现了4个编译器原理演示模块，展示了从源代码到可执行代码的完整编译过程：

1. **词法分析** (Lexer) - 将源代码分解为Token序列
2. **语法分析** (Parser) - 构建抽象语法树(AST)
3. **语义分析** (Semantic) - 符号表管理和语义检查
4. **代码生成** (CodeGen) - 生成虚拟机指令

## 快速开始

### 1. 启动服务器

```bash
# 克隆项目
git clone <repository-url>
cd live-code-platform

# 安装依赖
npm install

# 启动服务器
make run
```

### 2. 访问演示页面

在浏览器中访问以下页面：

- **词法分析**: http://localhost:8080/pages/lexer.html
- **语法分析**: http://localhost:8080/pages/parser.html  
- **语义分析**: http://localhost:8080/pages/sema.html
- **代码生成**: http://localhost:8080/pages/codegen.html

## 演示功能详解

### 📝 词法分析模块

**访问地址**: http://localhost:8080/pages/lexer.html

**功能说明**:
- 将输入的源代码分解为Token序列
- 识别关键字、标识符、数字、字符串、运算符等
- 显示每个Token的类型和值

**示例代码**:
```python
def greet(name):
    let message = "Hello, " + name + "!"
    print(message)
    return message

let user = "World"
greet(user)
```

**输出示例**:
```
=== 词法分析演示 ===
输入代码:
def greet(name): ...

=== Token序列分析 ===
[1] 行1: TOKEN_DEF
[2] 行1: TOKEN_NAME = "greet"
[3] 行1: '('
[4] 行1: TOKEN_NAME = "name"
[5] 行1: ')'
[6] 行1: ':'
...
```

### 🌳 语法分析模块

**访问地址**: http://localhost:8080/pages/parser.html

**功能说明**:
- 分析代码的语法结构
- 识别函数定义、条件语句、循环等结构
- 展示语法树的层次结构

**示例代码**:
```python
def fibonacci(n):
    if n <= 1:
        return n
    else:
        return fibonacci(n-1) + fibonacci(n-2)

def main():
    let count = 10
    for i 1, count:
        let result = fibonacci(i)
        print("fibonacci(" + str(i) + ") = " + str(result))
```

**输出示例**:
```
=== 语法分析演示 ===
输入代码: ...

=== 语法树分析 ===
行1: 函数定义语句
  函数名: fibonacci
  参数列表开始
    参数: n
  参数列表结束
行2: 条件语句 (if)
行3: 返回语句
...
```

### 🔍 语义分析模块

**访问地址**: http://localhost:8080/pages/sema.html

**功能说明**:
- 构建符号表，管理变量和函数定义
- 检查符号引用的有效性
- 分析作用域和变量生命周期
- 检测重复定义等语义错误

**示例代码**:
```python
def calculate_area(length, width):
    let area = length * width
    return area

def calculate_volume(length, width, height):
    let base_area = calculate_area(length, width)
    let volume = base_area * height
    return volume

let room_length = 10.5
let room_width = 8.0
let room_height = 3.0
```

**输出示例**:
```
=== 语义分析演示 ===
输入代码: ...

=== 语义分析过程 ===
1. 符号表构建:
  添加函数符号: calculate_area (行1, 作用域0)
  添加变量符号: area (行2, 作用域1)
  添加函数符号: calculate_volume (行5, 作用域0)
  ...

2. 符号引用检查:
  ✓ 符号引用: length (定义在行1, 类型: variable)
  ✓ 符号引用: width (定义在行1, 类型: variable)
  ✓ 符号引用: calculate_area (定义在行1, 类型: function)
  ...
```

### ⚙️ 代码生成模块

**访问地址**: http://localhost:8080/pages/codegen.html

**功能说明**:
- 生成虚拟机指令序列
- 分析代码结构并生成对应的字节码
- 提供代码优化建议
- 展示指令的详细信息

**示例代码**:
```python
def add(a, b):
    return a + b

def calculate():
    let x = 10
    let y = 20
    let sum = add(x, y)
    print("x + y = " + str(sum))
```

**输出示例**:
```
=== 代码生成演示 ===
输入代码: ...

=== 字节码生成过程 ===
1. 代码分析与指令生成:
  处理函数定义 (行1)
    生成指令: CALL (函数: add)
  处理变量声明 (行5)
    生成指令: LOAD 0 (值: 10)
    生成指令: STORE 0 (变量: x)
  ...

2. 生成的字节码:
地址  指令      参数1  参数2  注释
----  --------  -----  -----  ----
0000  CALL          0      0  Function: add
0001  LOAD         10      0  Load 10 -> x
0002  STORE         0      0  Store var[0] (x)
...

3. 代码优化分析:
  - 加载指令: 3 条
  - 存储指令: 3 条  
  - 运算指令: 1 条
  - 总指令数: 8 条
```

## 编译器原理教学

这4个模块完整展示了编译器的工作流程：

1. **词法分析**: 输入字符流 → Token流
2. **语法分析**: Token流 → 抽象语法树
3. **语义分析**: AST + 符号表 → 语义检查
4. **代码生成**: AST → 目标代码

每个模块都可以独立使用，也可以组合使用来理解完整的编译过程。

## 技术架构

- **前端**: HTML5 + JavaScript + CodeMirror
- **后端**: Losu 编程语言 C 实现
- **编译**: Emscripten (C → WebAssembly)
- **通信**: ccall 函数调用
- **输出**: stdout 流捕获

## 自定义扩展

### 添加新的示例代码

在 `www/assets/script/{module}/` 目录下添加 `.els` 文件：

```bash
# 词法分析示例
www/assets/script/lexer/my-demo.els

# 语法分析示例  
www/assets/script/parser/my-demo.els

# 语义分析示例
www/assets/script/sema/my-demo.els

# 代码生成示例
www/assets/script/codegen/my-demo.els
```

### 重新编译WASM模块

如果修改了C代码，需要重新编译：

```bash
# 安装 Emscripten SDK
# https://emscripten.org/docs/getting_started/downloads.html

# 构建所有模块
make build-demos

# 或手动构建单个模块
./build_demos.sh
```

## 常见问题

**Q: 为什么模块加载失败？**
A: 检查WASM文件是否存在，浏览器是否支持WebAssembly

**Q: 如何添加新的Token类型？**  
A: 修改 `losu/losu/syntax/token.h` 和对应的词法分析器代码

**Q: 能否支持其他编程语言？**
A: 可以，修改对应模块的解析逻辑即可

**Q: 如何获得更详细的分析信息？**
A: 修改C代码中的printf输出，重新编译WASM模块

## 贡献指南

欢迎提交问题和改进建议！

1. Fork 项目
2. 创建特性分支
3. 提交改动
4. 发起 Pull Request

## 许可证

MIT License - 详见 LICENSE 文件 