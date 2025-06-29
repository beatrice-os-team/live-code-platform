(function (mod) {
    if (typeof exports == "object" && typeof module == "object") // CommonJS
        mod(require("../../lib/codemirror"));
    else if (typeof define == "function" && define.amd) // AMD
        define(["../../lib/codemirror"], mod);
    else // Plain browser env
        mod(CodeMirror);
})(function (CodeMirror) {
    "use strict";

    CodeMirror.defineMode("ec2", function () {
        function words(l) {
            var obj = {};
            for (var i = 0; i < l.length; ++i) obj[l[i]] = true;
            return obj;
        }
        // var keywords = words(
        //     "算始 算终 定义 函始 函终 若始 若终 若另 若否 " +
        //     "当始 当终 跳出 继续 返回 未定义 真 假 或 且 非 空 " +
        //     "导入 声明"
        // );// 关键词高亮表
        var keywords = words([
            "and" ,"or" , "no" ,"true" ,"false" ,"pass" ,"if" ,"else" ,"elif" ,"def" ,"lambda"
            ,"..." ,"let" ,"global" ,"class" ,"return" ,"break" ,"continue" ,"yield" ,"while" ,"until"
            ,"for" ,"import" ,"async" ,"match" ,"case" ,"except" ,"raise"     
            ]);
        var atoms = words([
            "print" ,"input" ,"exit" ,"type" ,"int" ,"float" ,"str" ,"chr" ,"len" ,"eval" ,"exec" 
            ,"package" ,"assert" ,"dylib" ,"fs" ,"losu" ,"math" ,"os" ,"str" ,"time" 
            //"abs", "sqrt", "cbrt", "pow", "log", "log2", "log10", "PI", "E", "SQRT2"
        ]);//内置函数高亮表
        var op = words([
            "+" ,"-" ,"*" ,"/" ,"%" ,"=" ,"==" ,"**" ,"," ,"::" ,"." ,">" ,"<" ,"<=" ,">=" ,"->" ,"|" 
            ,"!=" ,"!" ,"(" ,")" ,"[" ,"]" ,"{" ,"}" ,"&" ,"|>" ,"-=" ,"+=" ,"*=" , "?" ,":" ,'"' 
            ,"'" , "#" ,
        ]);

        function tokenBase(stream, state) {
            var ch = stream.next();
            if (ch == "#") {
                stream.skipToEnd();
                return "comment";
            }
            if (ch == '"' || ch == "'") {
                state.tokenize = tokenString(ch);
                return state.tokenize(stream, state);
            }

            if (ch == '.' || ch == ',' || ch == ';' || ch == ':' || ch == '[' || ch == ']' || ch == '{' || ch == '}' || ch == '(' || ch == ')') {
                return "operator";
            }


            // if (isOperatorChar.test(ch)) {
            //     stream.eatWhile(isOperatorChar);
            //     return "operator";
            // }
            // 支持中文字符
            stream.eatWhile(/\p{L}\w*/u);
            var cur = stream.current();
            if (keywords.propertyIsEnumerable(cur)) return "keyword";
            if (atoms.propertyIsEnumerable(cur)) return "atom";
            if (op.propertyIsEnumerable(cur)) return "operator";

            if (/^\p{L}[\p{L}\d_]*$/u.test(ch)) { // 检查是否为合法的变量名起始字符，并允许后续的字母、数字和下划线
                stream.eatWhile(/[\p{L}\d_]/u); // 吃掉所有字母、数字和下划线
                return "variable"; // 返回变量名类型
            } else if (/\d/.test(ch)) { // 检查是否为数字
                stream.eatWhile(/\d/); // 吃掉所有数字
                return "number"; // 返回数字类型
            }
            return "variable";
        }

        function tokenString(quote) {
            return function (stream, state) {
                var escaped = false, next, end = false;
                while ((next = stream.next()) != null) {
                    if (next == quote && !escaped) { end = true; break; }
                    escaped = !escaped && next == "\\";
                }
                if (end || !escaped) state.tokenize = null;
                return "string";
            };
        }


        // Interface

        return {
            startState: function () {
                return { tokenize: null };
            },

            token: function (stream, state) {
                if (stream.eatSpace()) return null;
                var style = (state.tokenize || tokenBase)(stream, state);
                if (style == "comment" || style == "meta") return style;
                return style;
            },
            indent: function (state, textAfter) {
                // 每 四个空格或一个tab缩进一个层级
                return CodeMirror.Pass;

            },

            // 支持方括号补全
            autoCloseBrackets: "()[]{}''\"\""
        };
    });
    CodeMirror.defineMIME("text/x-ec2", "ec2");






});