#include <emscripten.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "losu.h"

EMSCRIPTEN_KEEPALIVE void parser_demo(const char* input) {
    if (!input || strlen(input) == 0) {
        printf("语法分析输入为空\n");
        return;
    }

    printf("=== 语法分析演示 ===\n");
    printf("输入代码:\n%s\n", input);
    printf("\n=== 语法树分析 ===\n");
    
    // 创建虚拟机
    losu_vm_t vm = vm_create(1024);
    if (!vm) {
        fprintf(stderr, "创建虚拟机失败\n");
        return;
    }

    vm_setargs(vm, 0, NULL);
    vm_setmodule(vm, LOSU_NMODULE, LOSU_MODULE);

    printf("开始语法分析...\n");
    
    // 简单的语法结构识别
    const char* p = input;
    int line = 1;
    int indent_level = 0;
    
    printf("语法树结构:\n");
    
    // 简单解析来展示语法结构
    while (*p) {
        // 跳过空格和tab
        while (*p && (*p == ' ' || *p == '\t')) {
            if (*p == '\t') indent_level++;
            p++;
        }
        
        if (*p == '\n') {
            line++;
            p++;
            indent_level = 0;
            continue;
        }
        
        // 识别语句类型
        if (strncmp(p, "def ", 4) == 0) {
            printf("%*s行%d: 函数定义语句\n", indent_level * 2, "", line);
            p += 4;
            
            // 提取函数名
            while (*p && *p == ' ') p++;
            char func_name[64];
            int i = 0;
            while (*p && *p != '(' && *p != ' ' && i < 63) {
                func_name[i++] = *p++;
            }
            func_name[i] = '\0';
            printf("%*s  函数名: %s\n", indent_level * 2, "", func_name);
            
            // 查找参数
            if (*p == '(') {
                printf("%*s  参数列表开始\n", indent_level * 2, "");
                p++;
                while (*p && *p != ')') {
                    if (*p != ' ' && *p != ',') {
                        char param[32];
                        int j = 0;
                        while (*p && *p != ',' && *p != ')' && *p != ' ' && j < 31) {
                            param[j++] = *p++;
                        }
                        param[j] = '\0';
                        if (strlen(param) > 0) {
                            printf("%*s    参数: %s\n", indent_level * 2, "", param);
                        }
                    } else {
                        p++;
                    }
                }
                if (*p == ')') {
                    printf("%*s  参数列表结束\n", indent_level * 2, "");
                    p++;
                }
            }
        } else if (strncmp(p, "if ", 3) == 0) {
            printf("%*s行%d: 条件语句 (if)\n", indent_level * 2, "", line);
            p += 3;
        } else if (strncmp(p, "else", 4) == 0) {
            printf("%*s行%d: 条件语句 (else)\n", indent_level * 2, "", line);
            p += 4;
        } else if (strncmp(p, "while ", 6) == 0) {
            printf("%*s行%d: 循环语句 (while)\n", indent_level * 2, "", line);
            p += 6;
        } else if (strncmp(p, "for ", 4) == 0) {
            printf("%*s行%d: 循环语句 (for)\n", indent_level * 2, "", line);
            p += 4;
        } else if (strncmp(p, "return", 6) == 0) {
            printf("%*s行%d: 返回语句\n", indent_level * 2, "", line);
            p += 6;
        } else if (strncmp(p, "let ", 4) == 0) {
            printf("%*s行%d: 变量声明语句\n", indent_level * 2, "", line);
            p += 4;
            
            // 提取变量名
            while (*p && *p == ' ') p++;
            char var_name[64];
            int i = 0;
            while (*p && *p != '=' && *p != ' ' && *p != '\n' && i < 63) {
                var_name[i++] = *p++;
            }
            var_name[i] = '\0';
            if (strlen(var_name) > 0) {
                printf("%*s  变量名: %s\n", indent_level * 2, "", var_name);
            }
        } else {
            // 其他语句（表达式、赋值等）
            char line_content[256];
            int i = 0;
            const char* line_start = p;
            while (*p && *p != '\n' && i < 255) {
                line_content[i++] = *p++;
            }
            line_content[i] = '\0';
            
            if (strlen(line_content) > 0) {
                // 检查是否包含赋值
                if (strchr(line_content, '=')) {
                    printf("%*s行%d: 赋值表达式: %s\n", indent_level * 2, "", line, line_content);
                } else if (strchr(line_content, '(') && strchr(line_content, ')')) {
                    printf("%*s行%d: 函数调用: %s\n", indent_level * 2, "", line, line_content);
                } else {
                    printf("%*s行%d: 表达式语句: %s\n", indent_level * 2, "", line, line_content);
                }
            }
        }
        
        // 跳到行尾
        while (*p && *p != '\n') p++;
        if (*p == '\n') {
            line++;
            p++;
            indent_level = 0;
        }
    }

    printf("\n=== 语法分析完成 ===\n");
    printf("总行数: %d\n", line - 1);

    // 清理资源
    vm_close(vm);
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