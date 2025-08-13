var this_html = {
    editor: CodeMirror.fromTextArea(document.getElementById('editor-code'), {
        mode: 'python',
        theme: 'material-palenight',
        lineNumbers: true,
        lineWrapping: true,
        autoCloseBrackets: true,
        matchBrackets: true,
        indentUnit: 4,
        tabSize: 4,
        extraKeys: {
            "Ctrl-Space": "autocomplete",
            "Ctrl-Enter": function() {
                this_html.editor_run();
            }
        }
    }),

    // 清空编辑器
    editor_clear() {
        this.editor.setValue('');
        document.getElementById('editor-result').innerHTML = "";
    },

    // 加载脚本文件
    loadscript(filename) {
        let url = `/assets/script/thread/${filename}`;
        fetch(url)
            .then(response => response.text())
            .then(data => {
                this.editor.setValue(data);
            })
            .catch(error => {
                console.error('加载脚本失败:', error);
                layui.layer.msg('加载脚本失败', {icon: 5});
            });
    },

    // FCFS演示
    demo_fcfs() {
        try {
            this.editor_clear();
            if (window.LosuThreadModule && window.LosuThreadModule._demo_fcfs) {
                window.LosuThreadModule._demo_fcfs();
            } else {
                layui.layer.msg('WASM模块未加载，请先运行一次代码', {icon: 5});
            }
        } catch (e) {
            console.error('FCFS演示失败:', e);
            layui.layer.msg('演示失败', {icon: 5});
        }
    },

    // 时间片轮转演示
    demo_rr() {
        try {
            this.editor_clear();
            if (window.LosuThreadModule && window.LosuThreadModule._demo_round_robin) {
                window.LosuThreadModule._demo_round_robin();
            } else {
                layui.layer.msg('WASM模块未加载，请先运行一次代码', {icon: 5});
            }
        } catch (e) {
            console.error('时间片轮转演示失败:', e);
            layui.layer.msg('演示失败', {icon: 5});
        }
    },

    // 切换主题
    toggle_theme() {
        // 主题切换逻辑可以后续添加
        layui.layer.msg('主题切换功能开发中', {icon: 6});
    },

    // 运行线程调度
    async editor_run() {
        try {
            let load = layui.layer.msg('线程调度演示进行中...', {
                icon: 16,
                shade: 0.01
            });
            
            // 获取输入代码
            const inputCode = this.editor.getValue();
            
            // 清空之前的结果
            document.getElementById('editor-result').innerHTML = '';
            
            // 使用线程调度演示模块
            let losuModule = window.LosuThreadModule;
            if (!losuModule) {
                // 尝试加载线程调度WASM模块
                if (typeof LosuThread === 'undefined') {
                    document.getElementById('editor-result').innerHTML += 
                        `<span style="color:red">WASM模块未加载，请刷新页面重试</span><br>`;
                    layui.layer.close(load);
                    return;
                }
                
                losuModule = await LosuThread({
                    print(text) {
                        document.getElementById('editor-result').innerHTML +=
                            `<span style="color:white">${text}</span><br>`;
                    },
                    printErr(text) {
                        document.getElementById('editor-result').innerHTML +=
                            `<span style="color:red">错误: ${text}</span><br>`;
                    },
                });
                
                // 缓存模块以供后续使用
                window.LosuThreadModule = losuModule;
            }
            
            // 直接调用导出的函数
            if (losuModule && losuModule._thread_demo) {
                try {
                    // 手动分配内存并转换字符串
                    const strLen = losuModule.lengthBytesUTF8(inputCode) + 1;
                    const strPtr = losuModule._malloc(strLen);
                    losuModule.stringToUTF8(inputCode, strPtr, strLen);
                    
                    // 调用线程调度演示函数
                    losuModule._thread_demo(strPtr);
                    
                    // 释放内存
                    losuModule._free(strPtr);
                } catch (threadError) {
                    console.warn('thread_demo函数调用失败，使用通用运行函数:', threadError);
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
            console.error('线程调度出错:', e);
            layui.layer.close(load);
            layui.layer.msg('线程调度执行失败', {icon: 5});
        }
    }
};

// 初始化layui组件
layui.use(['layer', 'util'], function(){
    var layer = layui.layer;
    var util = layui.util;
    
    // 页面加载完成后的初始化
    this_html.editor.setSize(null, 'auto');
});
