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
            let load = layui.layer.msg('语义分析进行中 ... ', {
                icon: 16,
                shade: 0.01
            });
            
            // 获取输入代码
            const inputCode = this.editor.getValue();
            if (!inputCode.trim()) {
                document.getElementById('editor-result').innerHTML = 
                    `<span style="color:orange">请输入一些代码进行语义分析</span><br>`;
                layui.layer.close(load);
                return;
            }
            
            // 清空之前的结果
            document.getElementById('editor-result').innerHTML = '';
            
            // 使用语义分析演示模块
            let losuModule = window.LosuSemaModule;
            if (!losuModule) {
                // 尝试加载语义分析WASM模块
                if (typeof LosuSema === 'undefined') {
                    document.getElementById('editor-result').innerHTML += 
                        `<span style="color:red">WASM模块未加载，请刷新页面重试</span><br>`;
                    layui.layer.close(load);
                    return;
                }
                
                losuModule = await LosuSema({
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
                window.LosuSemaModule = losuModule;
            }
            
            // 直接调用导出的函数
            if (losuModule && losuModule._sema_demo) {
                try {
                    // 手动分配内存并转换字符串
                    const strLen = losuModule.lengthBytesUTF8(inputCode) + 1;
                    const strPtr = losuModule._malloc(strLen);
                    losuModule.stringToUTF8(inputCode, strPtr, strLen);
                    
                    // 调用语义分析演示函数
                    losuModule._sema_demo(strPtr);
                    
                    // 释放内存
                    losuModule._free(strPtr);
                } catch (semaError) {
                    console.warn('sema_demo函数调用失败，使用通用运行函数:', semaError);
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
            console.error('语义分析出错:', e);
            document.getElementById('editor-result').innerHTML += 
                `<span style="color:red">语义分析出错: ${e.message}</span><br>`;
            layui.layer.close(load);
        }
    },
    async loadscript(url) {
        try {
            let res = await fetch('/assets/script/sema/' + url);
            if (!res.ok) {
                throw new Error(res.statusText);
            }
            let text = await res.text();
            this.editor.setValue(text);
        } catch (error) {
            this.editor.setValue("# 导入出错\n");
            console.error(error);
        }
    },
};

layui.util.on('lay-on', {
    iframe: async function () {
        // iframe 层
        try {
            let res = await fetch("/assets/markdown/sema.md");
            let text = "";
            if (res.status == 200) {
                text = await res.text();
            } else {
                text = "加载失败";
            }
            layer.open({
                type: 1,
                area: ['60%', '80%'], // 宽高
                shade: 0.5,
                content: text,
                title: "语义分析说明"
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
        content: "# 语义分析演示 \n使用Losu语言进行语义分析"
    });
})();