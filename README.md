# 基于洛书（Losu）编程语言与 Web Assembly 的编译与OS演示系统

[![Sync to GitLab](https://github.com/beatrice-os-team/live-code-platform/actions/workflows/mirror.yml/badge.svg)](https://github.com/beatrice-os-team/live-code-platform/actions/workflows/mirror.yml)
[![Dependabot Updates](https://github.com/beatrice-os-team/live-code-platform/actions/workflows/dependabot/dependabot-updates/badge.svg)](https://github.com/beatrice-os-team/live-code-platform/actions/workflows/dependabot/dependabot-updates)
[![CodeQL](https://github.com/beatrice-os-team/live-code-platform/actions/workflows/github-code-scanning/codeql/badge.svg)](https://github.com/beatrice-os-team/live-code-platform/actions/workflows/github-code-scanning/codeql)

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
|- index.js   # 启动服务
```
+ www
```
/
|- index.html       # 首页
|- assets/
    |- js/              # js
    |- css/             # css
    |- images/          # 图片
    |- fonts/           # 字体
    |- wasm/            # wasm 文件
    |- markdown/        # markdown 文件
    |- modules/         # 模块
|- pages/           # 功能页面 
    |- readme.html      # 总体介绍
    |- editor.html      # editor - 编辑器
    |- lexer.html       # lexer - 词法分析实验
    |- parser.html      # parser - 语法分析实验
    |- codegen.html     # codegen - 代码生成实验
    |- memory.html      # memory - 内存管理实验
    |- filesystem.html  # filesystem - 文件系统实验
    |- interrupt.html   # interrupt - 中断实验
    |- thread.html      # thread - 线程实验

```

+ index.js 
    - 基于 express 构建的 web 服务
    - 路由规则
        - 静态资源 `/`
        - markdown  `*.md`
        - api `/api/*`

