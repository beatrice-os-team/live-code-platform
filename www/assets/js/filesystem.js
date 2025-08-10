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
    filesystemModule: null, // 缓存的WASM模块
    
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

    async loadFilesystemModule() {
        if (this.filesystemModule) {
            return this.filesystemModule;
        }

        try {
            if (typeof LosuFilesystem === 'undefined') {
                throw new Error('文件系统WASM模块未加载');
            }

            this.filesystemModule = await LosuFilesystem({
                print(text) {
                    document.getElementById('editor-result').innerHTML +=
                        `<span style="color:white">` + text + `</span><br>`;
                },
                printErr(text) {
                    document.getElementById('editor-result').innerHTML +=
                        `<span style="color:red">` + text + `</span><br>`;
                },
            });

            return this.filesystemModule;
        } catch (error) {
            console.error('加载文件系统模块失败:', error);
            throw error;
        }
    },

    async callModuleFunction(functionName, ...args) {
        try {
            const module = await this.loadFilesystemModule();
            
            if (!module[functionName]) {
                throw new Error(`函数 ${functionName} 不存在`);
            }

            // 处理字符串参数
            const processedArgs = args.map(arg => {
                if (typeof arg === 'string') {
                    const strLen = module.lengthBytesUTF8(arg) + 1;
                    const strPtr = module._malloc(strLen);
                    module.stringToUTF8(arg, strPtr, strLen);
                    return strPtr;
                }
                return arg;
            });

            // 调用函数
            const result = module[functionName](...processedArgs);

            // 释放字符串内存
            processedArgs.forEach((arg, index) => {
                if (typeof args[index] === 'string') {
                    module._free(arg);
                }
            });

            return result;
        } catch (error) {
            console.error('调用模块函数失败:', error);
            document.getElementById('editor-result').innerHTML +=
                `<span style="color:red">❌ 调用失败: ${error.message}</span><br>`;
        }
    },

    async editor_clear() {
        this.editor.setValue("");
        document.getElementById('editor-result').innerHTML = "";
    },

    async editor_run() {
        try {
            let load = layui.layer.msg('文件系统分析进行中 ... ', {
                icon: 16,
                shade: 0.01
            });

            const inputCode = this.editor.getValue();
            if (!inputCode.trim()) {
                document.getElementById('editor-result').innerHTML = 
                    `<span style="color:orange">请输入一些代码进行文件系统演示</span><br>`;
                layui.layer.close(load);
                return;
            }

            // 清空之前的结果
            document.getElementById('editor-result').innerHTML = '';

            // 使用文件系统演示模块
            await this.callModuleFunction('_filesystem_demo', inputCode);

            layui.layer.close(load);
        } catch (error) {
            console.error('文件系统演示出错:', error);
            document.getElementById('editor-result').innerHTML += 
                `<span style="color:red">文件系统演示出错: ${error.message}</span><br>`;
            layui.layer.close && layui.layer.close(load);
        }
    },

    async filesystem_demo() {
        try {
            let load = layui.layer.msg('初始化文件系统演示 ... ', {
                icon: 16,
                shade: 0.01
            });

            document.getElementById('editor-result').innerHTML = '';
            
            const demoCode = `// 文件系统综合演示
import fs

// 这是一个展示文件系统各种操作的演示
print("=== 开始文件系统演示 ===")`;

            await this.callModuleFunction('_filesystem_demo', demoCode);

            layui.layer.close(load);
        } catch (error) {
            console.error('文件系统演示失败:', error);
            layui.layer.close && layui.layer.close(load);
        }
    },

    async demo_read_file() {
        const filepath = prompt('请输入要读取的文件路径:', '/demo/hello.txt');
        if (filepath) {
            document.getElementById('editor-result').innerHTML += 
                `<hr><span style="color:#5FB878">🔍 读取文件: ${filepath}</span><br>`;
            await this.callModuleFunction('_demo_fs_read', filepath);
        }
    },

    async demo_write_file() {
        const filepath = prompt('请输入文件路径:', '/demo/test.txt');
        if (filepath) {
            const content = prompt('请输入文件内容:', 'Hello, FileSystem!');
            if (content !== null) {
                document.getElementById('editor-result').innerHTML += 
                    `<hr><span style="color:#5FB878">✏️ 写入文件: ${filepath}</span><br>`;
                await this.callModuleFunction('_demo_fs_write', filepath, content);
            }
        }
    },

    async demo_stat_file() {
        const filepath = prompt('请输入文件路径:', '/demo/hello.txt');
        if (filepath) {
            document.getElementById('editor-result').innerHTML += 
                `<hr><span style="color:#5FB878">📊 获取文件信息: ${filepath}</span><br>`;
            await this.callModuleFunction('_demo_fs_stat', filepath);
        }
    },

    async demo_delete_file() {
        const filepath = prompt('请输入要删除的文件路径:', '/demo/test.txt');
        if (filepath) {
            const confirmed = confirm(`确定要删除文件 ${filepath} 吗？`);
            if (confirmed) {
                document.getElementById('editor-result').innerHTML += 
                    `<hr><span style="color:#FF5722">🗑️ 删除文件: ${filepath}</span><br>`;
                await this.callModuleFunction('_demo_fs_unlink', filepath);
            }
        }
    },

    async demo_list_dir() {
        const dirpath = prompt('请输入目录路径:', '/demo');
        if (dirpath) {
            document.getElementById('editor-result').innerHTML += 
                `<hr><span style="color:#5FB878">📂 列出目录: ${dirpath}</span><br>`;
            await this.callModuleFunction('_demo_fs_readdir', dirpath);
        }
    },

    async demo_make_dir() {
        const dirpath = prompt('请输入新目录路径:', '/demo/newdir');
        if (dirpath) {
            document.getElementById('editor-result').innerHTML += 
                `<hr><span style="color:#5FB878">📁 创建目录: ${dirpath}</span><br>`;
            await this.callModuleFunction('_demo_fs_mkdir', dirpath);
        }
    },

    async demo_rename_file() {
        const oldpath = prompt('请输入原文件路径:', '/demo/hello.txt');
        if (oldpath) {
            const newpath = prompt('请输入新文件路径:', '/demo/hello_renamed.txt');
            if (newpath) {
                document.getElementById('editor-result').innerHTML += 
                    `<hr><span style="color:#5FB878">🔄 重命名文件: ${oldpath} → ${newpath}</span><br>`;
                await this.callModuleFunction('_demo_fs_rename', oldpath, newpath);
            }
        }
    },

    async demo_stat_dir() {
        const dirpath = prompt('请输入目录路径:', '/demo');
        if (dirpath) {
            document.getElementById('editor-result').innerHTML += 
                `<hr><span style="color:#5FB878">📊 获取目录信息: ${dirpath}</span><br>`;
            await this.callModuleFunction('_demo_fs_stat', dirpath);
        }
    },

    async demo_rename_dir() {
        const oldpath = prompt('请输入原目录路径:', '/demo/subdir');
        if (oldpath) {
            const newpath = prompt('请输入新目录路径:', '/demo/renamed_subdir');
            if (newpath) {
                document.getElementById('editor-result').innerHTML += 
                    `<hr><span style="color:#5FB878">🔄 重命名目录: ${oldpath} → ${newpath}</span><br>`;
                await this.callModuleFunction('_demo_fs_rename', oldpath, newpath);
            }
        }
    },

    async demo_delete_dir() {
        const dirpath = prompt('请输入要删除的目录路径:', '/demo/subdir');
        if (dirpath) {
            const confirmed = confirm(`确定要删除目录 ${dirpath} 吗？\n注意：这将删除目录及其所有内容！`);
            if (confirmed) {
                document.getElementById('editor-result').innerHTML += 
                    `<hr><span style="color:#FF5722">🗑️ 删除目录: ${dirpath}</span><br>`;
                await this.callModuleFunction('_demo_fs_rmdir', dirpath);
            }
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

    async loadscript(url) {
        try {
            let res = await fetch('/assets/script/filesystem/' + url);
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
        try {
            let res = await fetch("/assets/markdown/filesystem.md");
            let text = "";
            if (res.status == 200) {
                text = await res.text();
            } else {
                text = "加载失败";
            }
            layer.open({
                type: 1,
                area: ['60%', '80%'],
                shade: 0.5,
                content: text,
                title: "文件系统说明"
            });
        } catch (e) {
            console.error(e);
        }
    },
});

// 初始化文件系统
(async () => {
    try {
        await this_html.loadFilesystemModule();
        console.log('文件系统模块加载成功');
        
        // 自动初始化文件系统，创建默认文件和目录
        document.getElementById('editor-result').innerHTML += 
            `<hr><span style="color:#5FB878">🚀 正在初始化文件系统...</span><br>`;
        await this_html.callModuleFunction('_filesystem_init');
        
        console.log('文件系统自动初始化完成');
    } catch (error) {
        console.error('文件系统模块加载或初始化失败:', error);
        document.getElementById('editor-result').innerHTML = 
            `<span style="color:red">⚠️ 文件系统模块加载失败，请检查WASM文件</span><br>`;
    }
})();
