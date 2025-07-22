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
            let load = layui.layer.msg('代码运行中 ... ', {
                icon: 16,
                shade: 0.01
            });;
            let losu = await LosuLiveCode({
                print(text) {
                    document.getElementById('editor-result').innerHTML +=
                        `<span style="color:white">` + text + `</span><br>`;
                },
                printErr(text) {
                    document.getElementById('editor-result').innerHTML +=
                        `<span style="color:red">` + text + `</span><br>`;
                },

            });
            losu.ccall('run', [], ['string'], [this.editor.getValue()]);
            layui.layer.close(load);
        } catch (e) {
            console.error(e);
        }
    },
    async loadscript(url) {
        try {
            let res = await fetch('/assets/script/lexer/' + url);
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
            let res = await fetch("/assets/markdown/lexer.md");
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
                title: "   "
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
        content: "# 你好 \n你好我的朋友"
    });
})();