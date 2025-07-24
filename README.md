# 基于洛书（Losu）编程语言与 Web Assembly 的编译与OS演示系统

[![Sync to GitLab](https://github.com/beatrice-os-team/live-code-platform/actions/workflows/mirror.yml/badge.svg)](https://github.com/beatrice-os-team/live-code-platform/actions/workflows/mirror.yml)
[![Dependabot Updates](https://github.com/beatrice-os-team/live-code-platform/actions/workflows/dependabot/dependabot-updates/badge.svg)](https://github.com/beatrice-os-team/live-code-platform/actions/workflows/dependabot/dependabot-updates)
[![CodeQL](https://github.com/beatrice-os-team/live-code-platform/actions/workflows/github-code-scanning/codeql/badge.svg)](https://github.com/beatrice-os-team/live-code-platform/actions/workflows/github-code-scanning/codeql)

## 初赛提交

初赛所需的全部内容请点击[连接](https://github.com/beatrice-os-team#%E5%88%9D%E8%B5%9B%E9%98%B6%E6%AE%B5)，包括项目、源代码分析、设计方案、进展汇报幻灯片、作品演示视频。

## 使用方式

1. 确保当前操作系统安装了 node 软件包。

2. 安装 npm 依赖。

```bash
npm install
```

3. 运行。

```bash
make run
```

访问 **http://localhost:8080**

## 编译器演示模块

本项目新增了4个编译器原理演示模块，展示编译过程的各个阶段：

### 1. 词法分析 (Lexer)
- **URL**: http://localhost:8080/pages/lexer.html
- **功能**: 将源代码分解为Token序列
- **演示**: 识别关键字、标识符、数字、字符串等语法元素

### 2. 语法分析 (Parser) 
- **URL**: http://localhost:8080/pages/parser.html
- **功能**: 根据语法规则构建抽象语法树(AST)
- **演示**: 分析函数定义、条件语句、循环结构等语法结构

### 3. 语义分析 (Semantic Analysis)
- **URL**: http://localhost:8080/pages/sema.html  
- **功能**: 构建符号表，进行类型检查和语义验证
- **演示**: 变量声明检查、符号引用验证、作用域分析

### 4. 代码生成 (Code Generation)
- **URL**: http://localhost:8080/pages/codegen.html
- **功能**: 生成目标代码或字节码指令
- **演示**: 虚拟机指令生成、代码优化建议

### 构建演示模块

如果需要重新编译演示模块（需要安装 Emscripten SDK）：

```bash
# 构建所有演示模块
make build-demos

# 或者构建并运行
make run-with-demos

# 清理生成的文件
make clean-demos

# 查看帮助
make help
```

### 技术实现

- **前端**: HTML5 + JavaScript + CodeMirror 编辑器
- **后端**: Losu编程语言 C 实现
- **编译**: Emscripten (C → WebAssembly)
- **通信**: WASM 模块通过 ccall 调用演示函数
- **输出**: 捕获 stdout 流在浏览器中实时显示

## 关键词
1. Web Assembly
    - 跨平台、可视化、高性能、云原生
    - 浏览器内运行
    - 快速迭代
2. 编译：编译原理
    - 词法分析
        - Token
        - Lexer
    - 语法分析
        - EBNF
        - Parser
        - AST
    - 语义分析
        - 软关键词
        - 符号表
    - 代码生成
        - 指令体系
        - 代码优化
        - 目标代码
    
3. OS：
    - 内存管理：
        - mem alloc
        - mem free
        - GC
    - 文件系统：
        - WASM FS
    - 任务管理：
        - 中断
        - 多线程


## 工程结构
+ root
```
/
|- assets/    # 相关资源
|- docs/      # 文档
|- images/    # 页面运行截图
|- server/    # 服务端代码
|- www/       # 页面代码
|- losu/      # Losu编程语言实现
|- index.js   # 启动服务
|- build_demos.sh # 演示模块构建脚本
```
+ www
```
/
|- index.html       # 首页
|- assets/
    |- js/              # js (包含4个演示模块的JS)
    |- css/             # css
    |- images/          # 图片
    |- fonts/           # 字体
    |- wasm/            # wasm 文件 (编译后的演示模块)
    |- markdown/        # markdown 文件
    |- script/          # 演示脚本 (4个模块的示例代码)
    |- modules/         # 模块
|- pages/           # 功能页面 
    |- readme.html      # 总体介绍
    |- editor.html      # editor - 编辑器
    |- lexer.html       # lexer - 词法分析实验 ✨
    |- parser.html      # parser - 语法分析实验 ✨
    |- sema.html        # sema - 语义分析实验 ✨
    |- codegen.html     # codegen - 代码生成实验 ✨
    |- memory.html      # memory - 内存管理实验
    |- filesystem.html  # filesystem - 文件系统实验
    |- interrupt.html   # interrupt - 中断实验
    |- thread.html      # thread - 线程实验

```

+ losu/ (编程语言实现)
```
/
|- include/         # 头文件
|- losu/           # 核心实现
|- lexer/          # 词法分析模块 ✨
|- parser/         # 语法分析模块 ✨  
|- sema/           # 语义分析模块 ✨
|- codegen/        # 代码生成模块 ✨
|- editor/         # 编辑器模块
|- main.c          # 主程序入口
```

+ index.js 
    - 基于 express 构建的 web 服务
    - 路由规则
        - 静态资源 `/`
        - markdown  `*.md`
        - api `/api/*`

