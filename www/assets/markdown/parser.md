# 基于洛书（Losu）编程语言与 Web Assembly 的编译与 OS 演示系统

---

## 项目简介

本项目基于洛书（Losu）编程语言与 Web Assembly 技术，打造一个集编程教学与操作系统原理演示于一体的交互式网站。用户可以在浏览器中编写 Losu 代码，体验编译、虚拟机运行、内存管理、线程调度、文件操作等底层 OS 功能的可视化过程，助力系统能力学习与创新实践。

本项目为 2025 年全国大学生计算机系统能力大赛-操作系统设计赛（全国）- OS功能挑战赛道参赛作品。

## 语法分析实验介绍

### 实验概述

语法分析（Syntax Analysis）是编译器的第二个阶段，也被称为解析（Parsing）。本实验通过洛书语言编译器的语法分析器，帮助学生理解语法分析的基本原理，学习如何将词法单元序列转换为抽象语法树（Abstract Syntax Tree, AST）。

### 学习目标

- **理解语法分析的作用**：掌握语法分析在编译过程中的地位和作用
- **掌握语法规则应用**：学习如何根据语法规则构建抽象语法树
- **了解洛书语言语法**：熟悉洛书语言的语法结构和特色语法元素
- **体验递归下降解析**：通过实际运行理解语法分析器的工作过程

### 实验内容

#### 1. 洛书语言语法结构

**基本语句**
- 表达式语句：`x + y`, `func(args)`
- 赋值语句：`let x = 10`, `x :: y + z`
- 控制流语句：`if`, `for`, `while`
- 函数定义：`def function_name(params):`

**类和对象**
- 类定义：`class ClassName:`
- 方法定义：`def method(self, args):`
- 构造函数：`def __init__(self):`
- 继承：`class Child(Parent):`

**函数和闭包**
- 普通函数：`def func(x, y): return x + y`
- Lambda 函数：`lambda x: x * 2`
- 异步函数：`async def async_func():`
- 生成器：`yield` 表达式

**表达式类型**
- 算术表达式：`a + b * c`
- 逻辑表达式：`x and y or z`
- 比较表达式：`a >= b`
- 函数调用：`func(arg1, arg2)`
- 属性访问：`obj.attr`
- 索引访问：`arr[index]`

#### 2. 抽象语法树结构

语法分析器将产生层次化的 AST 结构：

**程序结构**
```
Program
├── ClassDef (类定义)
│   ├── ClassName
│   ├── BaseClass (可选)
│   └── Methods[]
├── FunctionDef (函数定义)
│   ├── FunctionName  
│   ├── Parameters[]
│   └── Body[]
└── Statements[] (语句列表)
```

**表达式树**
```
BinaryOp (二元运算)
├── Left (左操作数)
├── Operator (运算符)
└── Right (右操作数)

FunctionCall (函数调用)
├── Function (函数表达式)
└── Arguments[] (参数列表)
```

#### 3. 示例代码解析

**简单表达式解析**
```losu
let result = 2 + 3 * 4
```
AST 结构：
```
AssignmentStmt
├── Variable: "result"
└── Expression: BinaryOp
    ├── Left: Literal(2)
    ├── Operator: "+"
    └── Right: BinaryOp
        ├── Left: Literal(3)
        ├── Operator: "*"
        └── Right: Literal(4)
```

**类定义解析**
```losu
class Io:
    def __init__(self):
        self.queue = []
        self.startptr = 0
        
    def push(self, item):
        self.queue[self.endptr] = item
        self.endptr = self.endptr + 1
```

**异步编程语法**
```losu
async def producer(io, id, f):
    for i in 1,10:
        io.push('producer: ' & str(id))
        yield

async producer(io, 1, lambda x: x*x)
```

**控制流结构**
```losu
for i in 1,10:
    if i % 2 == 0:
        print("偶数:", i)
    else:
        print("奇数:", i)
```

#### 4. 洛书语言特色语法

**软关键字系统**
- 根据上下文确定是否为关键字
- 例如：`yield` 在异步函数中是关键字，在其他地方可作为标识符

**管道运算符**
```losu
numbers |> map(lambda x: x * x) |> filter(lambda x: x > 10) |> list
```

**特殊赋值运算符**
```losu
let sum :: x + y    # :: 运算符用于特殊赋值
```

**多重赋值和解包**
```losu
let a, b = calculate(x, y)    # 多重赋值
```

### 操作步骤

1. **访问语法分析页面**：在主页侧边栏点击"语法分析"进入实验页面
2. **选择示例代码**：从预设的示例中选择，或编写自定义的洛书代码
3. **执行语法分析**：点击"解析"按钮，系统调用 WebAssembly 编译的语法分析器
4. **查看 AST 结构**：观察生成的抽象语法树，包括：
   - 节点类型和层次结构
   - 语法元素的组织关系
   - 运算符优先级的体现
5. **分析语法规则**：理解不同语法结构如何被解析为 AST 节点
6. **错误诊断**：尝试输入语法错误的代码，观察错误报告和恢复机制

### 实验重点

#### 理解语法分析过程
- **Token 到 AST 的转换**：观察词法单元如何组合成语法结构
- **语法规则的应用**：理解 EBNF 语法规则如何指导解析过程
- **运算符优先级**：观察表达式解析中优先级和结合性的处理
- **错误处理和恢复**：了解语法错误时的处理策略

#### 洛书语言特色解析
- **异步语法**：`async`/`yield` 关键字的解析
- **类和对象**：面向对象语法的解析
- **Lambda 表达式**：匿名函数的语法解析
- **管道运算符**：链式操作的语法处理

#### 抽象语法树理解
- **树形结构**：理解程序的层次化表示
- **节点类型**：不同语法元素对应的 AST 节点
- **语义信息**：AST 中包含的语义信息

### 语法错误处理

语法分析器具备完善的错误处理机制：

**常见语法错误**
- 括号不匹配：`print("hello"`
- 缺少冒号：`if x > 0 print(x)`
- 缩进错误：Python 风格的缩进要求
- 表达式不完整：`let x = +`

**错误恢复策略**
- 同步点恢复：在语句边界恢复解析
- 错误级联控制：避免一个错误产生多个错误报告
- 建议修复：提供可能的修复建议

### 技术实现

- **递归下降解析器**：手工编写的递归下降解析器
- **WebAssembly 执行**：高性能的浏览器内解析
- **可视化 AST**：图形化展示抽象语法树结构
- **实时解析**：代码修改时实时更新解析结果

### 扩展思考

1. **解析算法比较**：递归下降 vs LR vs LALR 解析器
2. **歧义处理**：如何处理语法歧义和解决冲突
3. **错误恢复策略**：不同错误恢复方法的优劣
4. **语法设计原则**：如何设计易于解析的语法

通过本实验，学生将深入理解编译器的语法分析阶段，掌握从词法单元到抽象语法树的转换过程，为后续的语义分析奠定基础。