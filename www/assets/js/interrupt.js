var this_html = {
    editor: CodeMirror.fromTextArea(document.getElementById('editor-code'), {
        lineNumbers: true,
        mode: "ec2",
        theme: "material-palenight",
        matchBrackets: true,
        indentUnit: 4,
        smartIndent: true,
        lineWrapping: true,
    }),
    theme: 1, // 0: light, 1: dark
    programRunning: false, // 程序是否在运行
    
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
            let load = layui.layer.msg('启动长时间程序中 ... ', {
                icon: 16,
                shade: 0.01
            });
            
            // 获取输入代码
            const inputCode = this.editor.getValue();
            
            // 清空之前的结果
            document.getElementById('editor-result').innerHTML = '';
            
            // 使用中断处理演示模块
            let losuModule = window.LosuInterruptModule;
            if (!losuModule) {
                // 尝试加载中断处理WASM模块
                if (typeof LosuInterrupt === 'undefined') {
                    document.getElementById('editor-result').innerHTML += 
                        `<span style="color:red">WASM模块未加载，请刷新页面重试</span><br>`;
                    layui.layer.close(load);
                    return;
                }
                
                losuModule = await LosuInterrupt({
                    print(text) {
                        document.getElementById('editor-result').innerHTML +=
                            `<span style="color:white">` + text + `</span><br>`;
                        // 自动滚动到底部
                        const resultDiv = document.getElementById('editor-result');
                        resultDiv.scrollTop = resultDiv.scrollHeight;
                    },
                    printErr(text) {
                        document.getElementById('editor-result').innerHTML +=
                            `<span style="color:red">` + text + `</span><br>`;
                        // 自动滚动到底部
                        const resultDiv = document.getElementById('editor-result');
                        resultDiv.scrollTop = resultDiv.scrollHeight;
                    },
                });
                
                // 缓存模块以供后续使用
                window.LosuInterruptModule = losuModule;
            }
            
            // 如果有代码输入，先显示代码
            if (inputCode.trim()) {
                document.getElementById('editor-result').innerHTML += 
                    `<span style="color:#61dafb">📝 用户代码:</span><br>`;
                document.getElementById('editor-result').innerHTML += 
                    `<span style="color:#98c379">${inputCode}</span><br><br>`;
            }
            
            // 启动长时间程序（默认100步，用户可以修改）
            const steps = this.getStepsFromCode(inputCode) || 100;
            this.programRunning = true;
            this.updateProgramButtons();
            
            // 初始化长时间程序（不会阻塞）
            if (losuModule && losuModule._start_long_program) {
                try {
                    losuModule._start_long_program(steps);
                    // 启动异步执行循环
                    this.startAsyncExecution();
                } catch (error) {
                    document.getElementById('editor-result').innerHTML +=
                        `<span style="color:red">❌ 程序初始化错误: ${error}</span><br>`;
                    this.programRunning = false;
                    this.updateProgramButtons();
                }
            }
            
            layui.layer.close(load);
            
        } catch (e) {
            console.error(e);
            document.getElementById('editor-result').innerHTML +=
                `<span style="color:red">❌ 系统错误: ${e.message}</span><br>`;
            this.programRunning = false;
            this.updateProgramButtons();
        }
    },

    // 从代码中提取步数参数
    getStepsFromCode(code) {
        const stepsMatch = code.match(/steps?\s*=\s*(\d+)/i);
        if (stepsMatch) {
            return parseInt(stepsMatch[1]);
        }
        return null;
    },

    // 更新程序控制按钮状态
    updateProgramButtons() {
        // 这里可以添加按钮状态更新逻辑
        // 例如禁用/启用某些按钮
    },

    // 异步执行程序步骤
    async startAsyncExecution() {
        const executeStep = async () => {
            try {
                const losuModule = await this.loadInterruptModule();
                if (!losuModule || !losuModule._execute_program_step) {
                    console.warn('execute_program_step function not available');
                    this.programRunning = false;
                    this.updateProgramButtons();
                    return;
                }

                // 执行一个程序步骤
                const result = losuModule._execute_program_step();
                
                // result: 1=继续执行, 0=程序结束
                if (result === 1 && this.programRunning) {
                    // 使用 requestAnimationFrame 来确保不阻塞UI
                    requestAnimationFrame(() => {
                        // 添加短暂延时避免执行过快
                        setTimeout(executeStep, 50);
                    });
                } else {
                    // 程序完成或被停止
                    this.programRunning = false;
                    this.updateProgramButtons();
                    document.getElementById('editor-result').innerHTML +=
                        `<span style="color:#98c379">📋 程序执行完成</span><br>`;
                }
                
            } catch (error) {
                console.error('执行程序步骤时出错:', error);
                this.programRunning = false;
                this.updateProgramButtons();
                document.getElementById('editor-result').innerHTML +=
                    `<span style="color:red">❌ 执行错误: ${error.message}</span><br>`;
            }
        };

        // 开始执行第一步
        executeStep();
    },

    // 中断处理相关功能
    async loadInterruptModule() {
        if (window.LosuInterruptModule) {
            return window.LosuInterruptModule;
        }

        try {
            if (typeof LosuInterrupt === 'undefined') {
                throw new Error('中断处理WASM模块未加载');
            }

            const module = await LosuInterrupt({
                print(text) {
                    document.getElementById('editor-result').innerHTML +=
                        `<span style="color:white">` + text + `</span><br>`;
                    // 自动滚动到底部
                    const resultDiv = document.getElementById('editor-result');
                    resultDiv.scrollTop = resultDiv.scrollHeight;
                },
                printErr(text) {
                    document.getElementById('editor-result').innerHTML +=
                        `<span style="color:red">` + text + `</span><br>`;
                    // 自动滚动到底部
                    const resultDiv = document.getElementById('editor-result');
                    resultDiv.scrollTop = resultDiv.scrollHeight;
                },
            });

            window.LosuInterruptModule = module;
            return module;
        } catch (error) {
            console.error('加载中断模块失败:', error);
            document.getElementById('editor-result').innerHTML +=
                `<span style="color:red">❌ 模块加载失败: ${error.message}</span><br>`;
            throw error;
        }
    },

    async callInterruptFunction(functionName, ...args) {
        try {
            const module = await this.loadInterruptModule();
            
            if (!module[functionName]) {
                throw new Error(`函数 ${functionName} 不存在`);
            }

            // 调用函数
            const result = module[functionName](...args);
            return result;
        } catch (error) {
            console.error('调用中断函数失败:', error);
            document.getElementById('editor-result').innerHTML +=
                `<span style="color:red">❌ 调用失败: ${error.message}</span><br>`;
        }
    },

    // 初始化中断系统
    async init_interrupt_system() {
        try {
            document.getElementById('editor-result').innerHTML += 
                `<span style="color:#61dafb">🚀 正在初始化中断系统...</span><br>`;
            await this.callInterruptFunction('_init_interrupt_system');
        } catch (error) {
            document.getElementById('editor-result').innerHTML +=
                `<span style="color:red">❌ 初始化失败: ${error.message}</span><br>`;
        }
    },

    // 启动长时间程序
    async start_long_program(steps = 100) {
        try {
            if (this.programRunning) {
                document.getElementById('editor-result').innerHTML += 
                    `<span style="color:orange">⚠️ 程序已在运行，请先停止当前程序</span><br>`;
                return;
            }

            document.getElementById('editor-result').innerHTML += 
                `<span style="color:#98c379">🚀 启动长时间程序 (${steps} 步)...</span><br>`;
            
            this.programRunning = true;
            this.updateProgramButtons();
            
            await this.callInterruptFunction('_start_long_program', steps);
            
            this.programRunning = false;
            this.updateProgramButtons();
            
        } catch (error) {
            this.programRunning = false;
            this.updateProgramButtons();
            document.getElementById('editor-result').innerHTML +=
                `<span style="color:red">❌ 启动程序失败: ${error.message}</span><br>`;
        }
    },

    // 触发中断
    async trigger_interrupt(interruptId) {
        try {
            const interruptNames = {
                1: "SIGINT (Ctrl+C)",
                2: "SIGTERM (程序终止)", 
                3: "SIGUSR1 (用户信号1)",
                4: "SIGUSR2 (用户信号2)",
                5: "TIMER (定时器中断)",
                6: "I/O (输入输出中断)",
                7: "MEMORY (内存中断)",
                8: "SYSCALL (系统调用中断)"
            };

            document.getElementById('editor-result').innerHTML += 
                `<span style="color:#e06c75">⚡ 触发中断: ${interruptNames[interruptId] || 'ID ' + interruptId}</span><br>`;
            
            await this.callInterruptFunction('_trigger_interrupt', parseInt(interruptId));
            
            // 添加视觉效果
            const resultDiv = document.getElementById('editor-result');
            resultDiv.classList.add('interrupt-triggered');
            setTimeout(() => {
                resultDiv.classList.remove('interrupt-triggered');
            }, 500);
            
        } catch (error) {
            document.getElementById('editor-result').innerHTML +=
                `<span style="color:red">❌ 触发中断失败: ${error.message}</span><br>`;
        }
    },

    // 强制停止程序
    async force_stop() {
        try {
            document.getElementById('editor-result').innerHTML += 
                `<span style="color:#ff6b6b">🛑 强制停止程序...</span><br>`;
            
            // 立即设置程序状态为停止，终止异步循环
            this.programRunning = false;
            this.updateProgramButtons();
            
            await this.callInterruptFunction('_force_stop_program');
            
            document.getElementById('editor-result').innerHTML +=
                `<span style="color:#98c379">✅ 程序已强制停止</span><br>`;
            
        } catch (error) {
            document.getElementById('editor-result').innerHTML +=
                `<span style="color:red">❌ 强制停止失败: ${error.message}</span><br>`;
        }
    },

    // 显示系统状态
    async show_status() {
        try {
            document.getElementById('editor-result').innerHTML += 
                `<span style="color:#61dafb">📊 显示系统状态:</span><br>`;
            await this.callInterruptFunction('_show_system_status');
        } catch (error) {
            document.getElementById('editor-result').innerHTML +=
                `<span style="color:red">❌ 显示状态失败: ${error.message}</span><br>`;
        }
    },

    // 读取日志
    async read_logs() {
        try {
            document.getElementById('editor-result').innerHTML += 
                `<span style="color:#c678dd">📚 读取中断日志:</span><br>`;
            await this.callInterruptFunction('_read_logs');
        } catch (error) {
            document.getElementById('editor-result').innerHTML +=
                `<span style="color:red">❌ 读取日志失败: ${error.message}</span><br>`;
        }
    },

    // 清除日志
    async clear_logs() {
        try {
            document.getElementById('editor-result').innerHTML += 
                `<span style="color:#56b6c2">🗑️ 清除日志文件...</span><br>`;
            await this.callInterruptFunction('_clear_logs');
        } catch (error) {
            document.getElementById('editor-result').innerHTML +=
                `<span style="color:red">❌ 清除日志失败: ${error.message}</span><br>`;
        }
    },

    // 演示文件系统操作
    async demo_filesystem() {
        try {
            document.getElementById('editor-result').innerHTML += 
                `<span style="color:#e5c07b">📁 文件系统操作演示:</span><br>`;
            await this.callInterruptFunction('_demo_filesystem_operations');
        } catch (error) {
            document.getElementById('editor-result').innerHTML +=
                `<span style="color:red">❌ 文件系统演示失败: ${error.message}</span><br>`;
        }
    },

    // 演示函数别名
    async demo_priorities() {
        await this.start_long_program(50);
    },

    async demo_exceptions() {
        await this.demo_filesystem();
    },

    async simulate_program() {
        await this.start_long_program(75);
    },

    async reset_system() {
        await this.force_stop();
        await this.clear_logs();
        await this.init_interrupt_system();
    },

    async loadscript(url) {
        try {
            let res = await fetch('/assets/script/interrupt/' + url);
            if (!res.ok) {
                throw new Error(res.statusText);
            }
            let text = await res.text();
            this.editor.setValue(text);
        } catch (error) {
            this.editor.setValue("# 导入出错\n");
            console.error(error);
        }
    }
};

// Layui事件绑定
layui.util.on('lay-on', {
    'iframe': function() {
        layer.open({
            type: 2,
            shade: 0.1,
            area: ['60%', '80%'],
            content: '/assets/markdown/index.html?interrupt.md',
            title: "中断处理 - 参考手册"
        });
    }
});

// 页面加载完成后的初始化
document.addEventListener('DOMContentLoaded', function() {
    console.log('🚀 中断处理演示模块已加载');
    console.log('💡 提示：点击"运行演示"启动长时间程序');
    console.log('⚡ 程序运行时可点击中断按钮进行中断测试');
    console.log('📚 建议：先点击"初始化系统"开始实验');
});