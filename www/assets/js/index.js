var this_html = {
    theme: 1, // 0: light, 1: dark
    toggle_theme() {
        try {
            if (this.theme == 1) {
                document.getElementById('layui_theme').removeAttribute('href');
                document.getElementById('highlight_theme').setAttribute('href', "/assets/lib/highlight@10.7.3/styles/atom-one-light.css");

                this.theme = 0;
            } else {
                document.getElementById('layui_theme').setAttribute('href', "/assets/lib/layui@2.11.4/css/layui-dark.css");
                document.getElementById('highlight_theme').setAttribute('href', "/assets/lib/highlight@10.7.3/styles/atom-one-dark.css");
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
                document.getElementById('markdown-text').innerHTML = text;
            } else {
                document.getElementById('markdown-text').innerHTML = "加载失败";
            }
        } catch (e) {
            console.error(e);
        }
    },

};

this_html.load_markdown('readme.md');