#include <emscripten.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "losu.h"

// Token名称映射表 - 简化版
const char* get_simple_token_name(int16_t token) {
    if (token < 256) {
        static char buf[8];
        if (token >= 32 && token <= 126) {
            snprintf(buf, sizeof(buf), "'%c'", (char)token);
        } else {
            snprintf(buf, sizeof(buf), "%d", token);
        }
        return buf;
    }
    
    // 常见Token的简单映射
    switch(token) {
        case 256: return "TOKEN_AND";
        case 257: return "TOKEN_OR";
        case 258: return "TOKEN_NOT";
        case 259: return "TOKEN_TRUE";
        case 260: return "TOKEN_FALSE";
        case 261: return "TOKEN_PASS";
        case 262: return "TOKEN_IF";
        case 263: return "TOKEN_ELSE";
        case 264: return "TOKEN_ELSEIF";
        case 265: return "TOKEN_DEF";
        case 266: return "TOKEN_LAMBDA";
        case 267: return "TOKEN_ARG";
        case 268: return "TOKEN_LET";
        case 269: return "TOKEN_GLOBAL";
        case 270: return "TOKEN_CLASS";
        case 271: return "TOKEN_RETURN";
        case 272: return "TOKEN_BREAK";
        case 273: return "TOKEN_CONTINUE";
        case 274: return "TOKEN_YIELD";
        case 275: return "TOKEN_WHILE";
        case 276: return "TOKEN_UNTIL";
        case 277: return "TOKEN_FOR";
        case 278: return "TOKEN_IMPORT";
        case 279: return "TOKEN_ASYNC";
        case 280: return "TOKEN_MATCH";
        case 281: return "TOKEN_CASE";
        case 282: return "TOKEN_EXCEPT";
        case 283: return "TOKEN_RAISE";
        case 284: return "TOKEN_EQ";
        case 285: return "TOKEN_GE";
        case 286: return "TOKEN_LE";
        case 287: return "TOKEN_NE";
        case 288: return "TOKEN_ASSIGN";
        case 289: return "TOKEN_POW";
        case 290: return "TOKEN_PIPE";
        case 291: return "TOKEN_NAME";
        case 292: return "TOKEN_NUMBER";
        case 293: return "TOKEN_STRING";
        case 294: return "TOKEN_EOZ";
        default: return "UNKNOWN_TOKEN";
    }
}

EMSCRIPTEN_KEEPALIVE void run(const char* input) {
    // 保持原有的完整运行功能
    losu_vm_t vm = vm_create(1024);
    if (!vm) {
        fprintf(stderr, "Failed to create Losu VM\n");
        return;
    }
    vm_setargs(vm, 0, NULL);
    vm_setmodule(vm, LOSU_NMODULE, LOSU_MODULE);
    if (vm_dostring(vm, input) == 0) {
        losu_ctype_bool b = 1;
        while (b) {
            b = 0;
            for (losu_object_coroutine_t coro = vm->coropool; coro;
                 coro = coro->next) {
                if (vm_await(vm, coro) != -1)
                    b = 1;
            }
        }
        gc_setthreshold(vm, 0);
        gc_collect(vm);
        printf("--------------------------------\n");
        printf("mem max: %.8g KB\n", (double)gc_getmemmax(vm) / 1024);
        printf("mem now: %.8g KB\n", (double)gc_getmemnow(vm) / 1024);
        printf("运行结束\n");
    } else {
        fprintf(stderr, "运行错误\n");
    }
    vm_close(vm);
} 

EMSCRIPTEN_KEEPALIVE void lexer_demo(const char* input) {
    if (!input || strlen(input) == 0) {
        printf("词法分析输入为空\n");
        return;
    }

    printf("=== 词法分析演示 ===\n");
    printf("输入代码:\n%s\n", input);
    printf("\n=== Token序列分析 ===\n");
    
    // 创建虚拟机
    losu_vm_t vm = vm_create(1024);
    if (!vm) {
        fprintf(stderr, "创建虚拟机失败\n");
        return;
    }

    vm_setargs(vm, 0, NULL);
    vm_setmodule(vm, LOSU_NMODULE, LOSU_MODULE);

    // 尝试解析代码获取词法信息
    printf("开始词法分析...\n");
    
    // 简单的字符扫描来模拟词法分析输出
    int line = 1;
    int token_count = 0;
    const char* p = input;
    char current_token[256];
    int token_pos = 0;
    
    while (*p) {
        char c = *p;
        
        if (c == '\n') {
            if (token_pos > 0) {
                current_token[token_pos] = '\0';
                token_count++;
                printf("[%d] 行%d: TOKEN_NAME = \"%s\"\n", token_count, line, current_token);
                token_pos = 0;
            }
            line++;
        } else if (c == ' ' || c == '\t') {
            if (token_pos > 0) {
                current_token[token_pos] = '\0';
                token_count++;
                
                // 简单的关键字识别
                if (strcmp(current_token, "def") == 0) {
                    printf("[%d] 行%d: TOKEN_DEF\n", token_count, line);
                } else if (strcmp(current_token, "if") == 0) {
                    printf("[%d] 行%d: TOKEN_IF\n", token_count, line);
                } else if (strcmp(current_token, "else") == 0) {
                    printf("[%d] 行%d: TOKEN_ELSE\n", token_count, line);
                } else if (strcmp(current_token, "while") == 0) {
                    printf("[%d] 行%d: TOKEN_WHILE\n", token_count, line);
                } else if (strcmp(current_token, "for") == 0) {
                    printf("[%d] 行%d: TOKEN_FOR\n", token_count, line);
                } else if (strcmp(current_token, "return") == 0) {
                    printf("[%d] 行%d: TOKEN_RETURN\n", token_count, line);
                } else if (strcmp(current_token, "let") == 0) {
                    printf("[%d] 行%d: TOKEN_LET\n", token_count, line);
                } else if (strcmp(current_token, "true") == 0) {
                    printf("[%d] 行%d: TOKEN_TRUE\n", token_count, line);
                } else if (strcmp(current_token, "false") == 0) {
                    printf("[%d] 行%d: TOKEN_FALSE\n", token_count, line);
                } else if (strspn(current_token, "0123456789.") == strlen(current_token)) {
                    printf("[%d] 行%d: TOKEN_NUMBER = %s\n", token_count, line, current_token);
                } else {
                    printf("[%d] 行%d: TOKEN_NAME = \"%s\"\n", token_count, line, current_token);
                }
                token_pos = 0;
            }
        } else if (c == '(' || c == ')' || c == '{' || c == '}' || c == '[' || c == ']' ||
                   c == '+' || c == '-' || c == '*' || c == '/' || c == '=' || c == '<' || c == '>' ||
                   c == ',' || c == ';' || c == ':' || c == '.' || c == '!' || c == '&' || c == '|') {
            if (token_pos > 0) {
                current_token[token_pos] = '\0';
                token_count++;
                printf("[%d] 行%d: TOKEN_NAME = \"%s\"\n", token_count, line, current_token);
                token_pos = 0;
            }
            token_count++;
            printf("[%d] 行%d: '%c'\n", token_count, line, c);
        } else if (c == '"') {
            // 处理字符串
            if (token_pos > 0) {
                current_token[token_pos] = '\0';
                token_count++;
                printf("[%d] 行%d: TOKEN_NAME = \"%s\"\n", token_count, line, current_token);
                token_pos = 0;
            }
            
            p++; // 跳过开始的引号
            char string_content[256];
            int str_pos = 0;
            while (*p && *p != '"' && str_pos < 255) {
                string_content[str_pos++] = *p++;
            }
            string_content[str_pos] = '\0';
            
            if (*p == '"') {
                token_count++;
                printf("[%d] 行%d: TOKEN_STRING = \"%s\"\n", token_count, line, string_content);
            }
        } else {
            if (token_pos < 255) {
                current_token[token_pos++] = c;
            }
        }
        
        p++;
    }
    
    // 处理最后一个token
    if (token_pos > 0) {
        current_token[token_pos] = '\0';
        token_count++;
        printf("[%d] 行%d: TOKEN_NAME = \"%s\"\n", token_count, line, current_token);
    }

    printf("\n=== 词法分析完成 ===\n");
    printf("总共识别了 %d 个Token\n", token_count);
    printf("总行数: %d\n", line);

    // 清理资源
    vm_close(vm);
    run(input);
}
