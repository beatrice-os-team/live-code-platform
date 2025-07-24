#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "include/losu.h"
#include "include/losu_syntax.h"
#include "losu/syntax/token.h"

// Token名称映射表
const char* token_names[512] = {0}; // 初始化为NULL

void init_token_names() {
    token_names[TOKEN_AND] = "TOKEN_AND";
    token_names[TOKEN_OR] = "TOKEN_OR"; 
    token_names[TOKEN_NOT] = "TOKEN_NOT";
    token_names[TOKEN_TRUE] = "TOKEN_TRUE";
    token_names[TOKEN_FALSE] = "TOKEN_FALSE";
    token_names[TOKEN_PASS] = "TOKEN_PASS";
    token_names[TOKEN_IF] = "TOKEN_IF";
    token_names[TOKEN_ELSE] = "TOKEN_ELSE";
    token_names[TOKEN_ELSEIF] = "TOKEN_ELSEIF";
    token_names[TOKEN_DEF] = "TOKEN_DEF";
    token_names[TOKEN_LAMBDA] = "TOKEN_LAMBDA";
    token_names[TOKEN_ARG] = "TOKEN_ARG";
    token_names[TOKEN_LET] = "TOKEN_LET";
    token_names[TOKEN_GLOBAL] = "TOKEN_GLOBAL";
    token_names[TOKEN_CLASS] = "TOKEN_CLASS";
    token_names[TOKEN_RETURN] = "TOKEN_RETURN";
    token_names[TOKEN_BREAK] = "TOKEN_BREAK";
    token_names[TOKEN_CONTINUE] = "TOKEN_CONTINUE";
    token_names[TOKEN_YIELD] = "TOKEN_YIELD";
    token_names[TOKEN_WHILE] = "TOKEN_WHILE";
    token_names[TOKEN_UNTIL] = "TOKEN_UNTIL";
    token_names[TOKEN_FOR] = "TOKEN_FOR";
    token_names[TOKEN_IMPORT] = "TOKEN_IMPORT";
    token_names[TOKEN_ASYNC] = "TOKEN_ASYNC";
    token_names[TOKEN_MATCH] = "TOKEN_MATCH";
    token_names[TOKEN_CASE] = "TOKEN_CASE";
    token_names[TOKEN_EXCEPT] = "TOKEN_EXCEPT";
    token_names[TOKEN_RAISE] = "TOKEN_RAISE";
    token_names[TOKEN_EQ] = "TOKEN_EQ";
    token_names[TOKEN_GE] = "TOKEN_GE";
    token_names[TOKEN_LE] = "TOKEN_LE";
    token_names[TOKEN_NE] = "TOKEN_NE";
    token_names[TOKEN_ASSIGN] = "TOKEN_ASSIGN";
    token_names[TOKEN_POW] = "TOKEN_POW";
    token_names[TOKEN_PIPE] = "TOKEN_PIPE";
    token_names[TOKEN_NAME] = "TOKEN_NAME";
    token_names[TOKEN_NUMBER] = "TOKEN_NUMBER";
    token_names[TOKEN_STRING] = "TOKEN_STRING";
    token_names[TOKEN_EOZ] = "TOKEN_EOZ";
}

const char* get_token_name(int16_t token) {
    if (token < 256) {
        static char buf[8];
        if (token >= 32 && token <= 126) {
            snprintf(buf, sizeof(buf), "'%c'", (char)token);
        } else {
            snprintf(buf, sizeof(buf), "%d", token);
        }
        return buf;
    }
    
    if (token < 512 && token_names[token]) {
        return token_names[token];
    }
    
    return "UNKNOWN_TOKEN";
}

void lexer_analyze(const char* input) {
    if (!input || strlen(input) == 0) {
        printf("词法分析输入为空\n");
        return;
    }

    // 初始化Token名称表
    init_token_names();

    printf("=== 词法分析开始 ===\n");
    printf("输入代码:\n%s\n", input);
    printf("\n=== Token序列 ===\n");
    
    // 创建虚拟机
    losu_vm_t vm = vm_create(1024);
    if (!vm) {
        fprintf(stderr, "Failed to create Losu VM\n");
        return;
    }

    // 设置IO
    losu_syntax_io io = {0};
    __losu_syntax_io_openS(&io, input, strlen(input), "lexer_demo");

    // 创建词法分析器
    losu_syntax_lex lex = {0};
    lex.vm = vm;
    lex.io = &io;
    lex.tkahead.token = TOKEN_EOZ;
    lex.fs = NULL;
    lex.lastline = lex.linenumber = 1;
    lex.current = losu_syntax_io_getc(&io);

    // 初始化词法分析器
    losu_syntax_lex_init(&lex);

    int token_count = 0;
    losu_syntax_tkvalue tkvalue;
    int16_t token;

    // 开始词法分析
    while ((token = losu_syntax_lex_next(&lex, &tkvalue)) != TOKEN_EOZ) {
        token_count++;
        printf("[%d] 行%d: %-15s", token_count, lex.linenumber, get_token_name(token));
        
        // 输出Token值
        switch (token) {
            case TOKEN_NAME:
                if (tkvalue.s && tkvalue.s->str) {
                    printf(" = \"%s\"", tkvalue.s->str);
                }
                break;
            case TOKEN_STRING:
                if (tkvalue.s && tkvalue.s->str) {
                    printf(" = \"%s\"", tkvalue.s->str);
                }
                break;
            case TOKEN_NUMBER:
                printf(" = %g", tkvalue.num);
                break;
            default:
                break;
        }
        printf("\n");
        
        // 防止无限循环
        if (token_count > 1000) {
            printf("警告: Token数量超过1000，停止分析\n");
            break;
        }
    }

    printf("\n=== 词法分析结束 ===\n");
    printf("总共识别了 %d 个Token\n", token_count);

    // 清理资源
    vm_close(vm);
} 