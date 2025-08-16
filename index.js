const express = require('express');
const fs = require('fs');
const path = require('path');
const marked = require('marked');
const hljs = require('highlight.js'); // 引入 highlight.js

const app = express();
const PORT = 8086;

app.post('/api/markdown', function (req, res) {
  const  markdown  = req.body;
  console.log(markdown);
  res.send('ok');
});

app.get('*.md', (req, res) => {
  try {
    const filePath = req.path;
    const mdFilePath = path.join(__dirname, 'www', filePath);
    const renderer = new marked.Renderer();
    renderer.code = function (code) {
      const validLanguage = hljs.getLanguage(code.lang) ? code.lang : 'plaintext';
      if (validLanguage === 'losu')
        validLanguage = 'python';
      const highlightedCode = hljs.highlight(code.text, { language: validLanguage }).value;
      return `<fieldset class="layui-elem-field"><pre><code class="layui-field-box hljs ${validLanguage}">${highlightedCode}</code></pre></fieldset>`;
    };
    const data = fs.readFileSync(mdFilePath, 'utf8');
    if (typeof data !== 'string') {
      throw new Error('File content is not a string');
    }

    const htmlContent = marked.parse(data, {
      gfm: false,
      tables: true,
      breaks: true,
      pedantic: false,
      smartLists: true,
      smartypants: true,
      renderer: renderer,
    });

    res.send(htmlContent);

  } catch (e) {
    console.error(e);
    res.status(502).send('502 Bad Gateway');
  }
});

app.use('/', express.static('www'));

// 捕获所有未匹配的路由
app.use((req, res, next) => {
  try {
    res.sendFile(path.join(__dirname, 'www', 'error.html'));
  } catch (e) {
    res.status(404).send('Server Error - 404 Not Found');
  }
});

app.listen(PORT, () => {
  console.log(`Server is running on http://localhost:${PORT}`);
});