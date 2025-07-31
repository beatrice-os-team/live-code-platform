var this_html = {
    editor: CodeMirror.fromTextArea(document.getElementById('editor-code'), {
        lineNumbers: true,
        mode: "ec2",
        theme: "material-palenight",
        matchBrackets: true,
        indentUnit: 4, // 每个缩进级别使用 4 个空格
        smartIndent: true, // 启用智能缩进
        lineWrapping: true,
    }),
    theme: 1, // 0: light, 1: dark
    toggle_theme() {
        try {
            if (this.theme == 1) {
                document.getElementById('layui_theme').removeAttribute('href');
                document.getElementById('highlight_theme').setAttribute('href', "/assets/lib/highlight@10.7.3/styles/atom-one-light.css");
                this.editor.setOption("theme", "solarized light");
                this.theme = 0;
            } else {
                document.getElementById('layui_theme').setAttribute('href', "/assets/lib/layui@2.11.4/css/layui-dark.css");
                document.getElementById('highlight_theme').setAttribute('href', "/assets/lib/highlight@10.7.3/styles/atom-one-dark.css");
                this.editor.setOption("theme", "material-palenight");
                this.theme = 1;
            }
        } catch (e) {
            console.error(e);
        }
    },
    async load_markdown(url) {
        try {
            let res = await fetch("/assets/markdown/" + url);
            if (res.status == 200) {
                let text = await res.text();
                return text;
            } else {
                return "加载失败";
            }
        } catch (e) {
            console.error(e);
            return "加载失败";
        }
    },
    editor_clear() {
        this.editor.setValue("");
        document.getElementById('editor-result').innerHTML = "";

    },
    async editor_run() {
        try {
            let load = layui.layer.msg('内存管理演示进行中 ... ', {
                icon: 16,
                shade: 0.01
            });
            
            // 获取输入代码
            const inputCode = this.editor.getValue();
            
            // 清空之前的结果
            document.getElementById('editor-result').innerHTML = '';
            
            // 使用内存管理演示模块
            let losuModule = window.LosuMemoryModule;
            if (!losuModule) {
                // 尝试加载内存管理WASM模块
                if (typeof LosuMemory === 'undefined') {
                    document.getElementById('editor-result').innerHTML += 
                        `<span style="color:red">WASM模块未加载，请刷新页面重试</span><br>`;
                    layui.layer.close(load);
                    return;
                }
                
                losuModule = await LosuMemory({
                    print(text) {
                        document.getElementById('editor-result').innerHTML +=
                            `<span style="color:white">` + text + `</span><br>`;
                    },
                    printErr(text) {
                        document.getElementById('editor-result').innerHTML +=
                            `<span style="color:red">` + text + `</span><br>`;
                    },
                });
                
                // 缓存模块以供后续使用
                window.LosuMemoryModule = losuModule;
            }
            
            // 直接调用导出的函数
            if (losuModule && losuModule._memory_demo) {
                try {
                    // 手动分配内存并转换字符串
                    const strLen = losuModule.lengthBytesUTF8(inputCode) + 1;
                    const strPtr = losuModule._malloc(strLen);
                    losuModule.stringToUTF8(inputCode, strPtr, strLen);
                    
                    // 调用内存管理演示函数
                    losuModule._memory_demo(strPtr);
                    
                    // 释放内存
                    losuModule._free(strPtr);
                } catch (memoryError) {
                    console.warn('memory_demo函数调用失败，使用通用运行函数:', memoryError);
                    try {
                        // 回退到通用运行函数
                        const strLen = losuModule.lengthBytesUTF8(inputCode) + 1;
                        const strPtr = losuModule._malloc(strLen);
                        losuModule.stringToUTF8(inputCode, strPtr, strLen);
                        losuModule._run(strPtr);
                        losuModule._free(strPtr);
                    } catch (runError) {
                        console.error('运行失败:', runError);
                        document.getElementById('editor-result').innerHTML += 
                            `<span style="color:red">运行失败: ${runError.message}</span><br>`;
                    }
                }
            } else {
                document.getElementById('editor-result').innerHTML += 
                    `<span style="color:red">WASM模块函数不可用，请检查模块是否正确加载</span><br>`;
            }
            
            layui.layer.close(load);
        } catch (e) {
            console.error('内存管理演示出错:', e);
            document.getElementById('editor-result').innerHTML += 
                `<span style="color:red">内存管理演示出错: ${e.message}</span><br>`;
            layui.layer.close(load);
        }
    },
    async loadscript(url) {
        try {
            let res = await fetch('/assets/script/memory/' + url);
            if (!res.ok) {
                throw new Error(res.statusText);
            }
            let text = await res.text();
            this.editor.setValue(text);
        } catch (error) {
            this.editor.setValue("# 导入出错\n# 示例脚本未找到，显示默认内容\n\n# 内存分配示例\ndef test_memory():\n    # 创建一些变量来测试内存分配\n    let arr = [1, 2, 3, 4, 5]\n    let str = \"Hello, Memory Management!\"\n    let num = 42\n    \n    # 这些变量的内存分配会被追踪\n    return arr, str, num\n\n# 调用测试函数\ntest_memory()\n");
            console.error(error);
        }
    },
};

layui.util.on('lay-on', {
    iframe: async function () {
        // iframe 层
        try {
            let res = await fetch("/assets/markdown/memory.md");
            let text = "";
            if (res.status == 200) {
                text = await res.text();
            } else {
                text = `# 内存管理演示说明

## 功能介绍

本模块演示Losu编程语言的内存管理机制，包括：

### 1. 内存分配与释放
- 展示基本的内存分配过程
- 演示内存释放和回收
- 跟踪内存块的生命周期

### 2. Losu虚拟机内存管理
- 使用Losu内置内存分配器
- 实时监控VM内存使用情况
- 展示内存分配的详细信息

### 3. 垃圾回收机制
- 触发垃圾回收过程
- 观察GC前后的内存变化
- 演示内存阈值管理

### 4. 代码执行内存分析
- 分析用户代码的内存使用
- 展示代码执行前后的内存状态
- 提供内存优化建议

## 使用方法

1. 在代码编辑器中输入Losu代码
2. 点击"运行演示"按钮
3. 观察右侧的内存管理演示结果
4. 可以选择示例代码进行测试

## 示例代码

- **内存分配示例**: 演示基本的变量创建和内存分配
- **垃圾回收示例**: 创建大量对象触发GC
- **内存优化示例**: 展示内存优化技巧

`;
            }
            layer.open({
                type: 1,
                area: ['60%', '80%'], // 宽高
                shade: 0.5,
                content: `<div style="padding: 20px; color: white; background: #2b2b2b; height: 100%; overflow-y: auto;"><pre style="white-space: pre-wrap; font-family: 'Courier New', monospace;">${text}</pre></div>`,
                title: "内存管理演示说明"
            });
        } catch (e) {
            console.error(e);
        }
    },
});

(async () => {
    let res = await fetch('/api/markdown', {
        method: 'POST',
        headers: {
            'Content-Type': 'plain/text'
        },
        content: "# 内存管理演示 \n使用Losu语言进行内存管理演示"
    });
})();