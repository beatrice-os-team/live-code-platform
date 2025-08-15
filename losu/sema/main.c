#include <emscripten.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#include "losu.h"

// 简单的符号表结构
typedef struct {
    char name[64];
    char type[32];
    int line;
    int scope_level;
} Symbol;

Symbol symbol_table[100];
int symbol_count = 0;
int current_scope = 0;

void add_symbol(const char* name, const char* type, int line) {
    if (symbol_count < 100) {
        strncpy(symbol_table[symbol_count].name, name, 63);
        strncpy(symbol_table[symbol_count].type, type, 31);
        symbol_table[symbol_count].line = line;
        symbol_table[symbol_count].scope_level = current_scope;
        symbol_count++;
    }
}

Symbol* find_symbol(const char* name) {
    for (int i = symbol_count - 1; i >= 0; i--) {
        if (strcmp(symbol_table[i].name, name) == 0) {
            return &symbol_table[i];
        }
    }
    return NULL;
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

EMSCRIPTEN_KEEPALIVE void sema_demo(const char* input) {
    if (!input || strlen(input) == 0) {
        printf("语义分析输入为空\n");
        return;
    }

    printf("=== 语义分析演示 ===\n");
    printf("输入代码:\n%s\n", input);
    printf("\n=== 语义分析过程 ===\n");
    
    // 创建虚拟机
    losu_vm_t vm = vm_create(1024);
    if (!vm) {
        fprintf(stderr, "创建虚拟机失败\n");
        return;
    }

    vm_setargs(vm, 0, NULL);
    vm_setmodule(vm, LOSU_NMODULE, LOSU_MODULE);

    // 重置符号表
    symbol_count = 0;
    current_scope = 0;

    printf("开始语义分析...\n");
    printf("\n1. 符号表构建:\n");
    
    // 简单的语义分析
    const char* p = input;
    int line = 1;
    
    while (*p) {
        // 跳过空格
        while (*p && (*p == ' ' || *p == '\t')) p++;
        
        if (*p == '\n') {
            line++;
            p++;
            continue;
        }
        
        // 函数定义
        if (strncmp(p, "def ", 4) == 0) {
            p += 4;
            while (*p && *p == ' ') p++;
            
            char func_name[64];
            int i = 0;
            while (*p && *p != '(' && *p != ' ' && i < 63) {
                func_name[i++] = *p++;
            }
            func_name[i] = '\0';
            
            if (strlen(func_name) > 0) {
                add_symbol(func_name, "function", line);
                printf("  添加函数符号: %s (行%d, 作用域%d)\n", func_name, line, current_scope);
                current_scope++; // 进入函数作用域
            }
        }
        // 变量声明
        else if (strncmp(p, "let ", 4) == 0) {
            p += 4;
            while (*p && *p == ' ') p++;
            
            char var_name[64];
            int i = 0;
            while (*p && *p != '=' && *p != ' ' && *p != '\n' && i < 63) {
                var_name[i++] = *p++;
            }
            var_name[i] = '\0';
            
            if (strlen(var_name) > 0) {
                // 检查是否重复定义
                Symbol* existing = find_symbol(var_name);
                if (existing && existing->scope_level == current_scope) {
                    printf("  ⚠️  警告: 变量 '%s' 在行%d重复定义 (首次定义在行%d)\n", 
                           var_name, line, existing->line);
                } else {
                    add_symbol(var_name, "variable", line);
                    printf("  添加变量符号: %s (行%d, 作用域%d)\n", var_name, line, current_scope);
                }
            }
        }
        
        // 跳到行尾
        while (*p && *p != '\n') p++;
        if (*p == '\n') {
            line++;
            p++;
        }
    }
    
    printf("\n2. 符号引用检查:\n");
    
    // 第二遍扫描：检查符号引用
    p = input;
    line = 1;
    current_scope = 0;
    
    while (*p) {
        // 跳过空格
        while (*p && (*p == ' ' || *p == '\t')) p++;
        
        if (*p == '\n') {
            line++;
            p++;
            continue;
        }
        
        // 检查变量使用
        if (isalpha(*p) || *p == '_') {
            char identifier[64];
            int i = 0;
            const char* start = p;
            
            while (*p && (isalnum(*p) || *p == '_') && i < 63) {
                identifier[i++] = *p++;
            }
            identifier[i] = '\0';
            
            // 跳过关键字
            if (strcmp(identifier, "def") == 0 || strcmp(identifier, "let") == 0 ||
                strcmp(identifier, "if") == 0 || strcmp(identifier, "else") == 0 ||
                strcmp(identifier, "while") == 0 || strcmp(identifier, "for") == 0 ||
                strcmp(identifier, "return") == 0) {
                // 跳过关键字
            } else if (strlen(identifier) > 0) {
                Symbol* sym = find_symbol(identifier);
                if (sym) {
                    printf("  ✓ 符号引用: %s (定义在行%d, 类型: %s)\n", 
                           identifier, sym->line, sym->type);
                } else {
                    printf("  ❌ 错误: 未定义的符号 '%s' (行%d)\n", identifier, line);
                }
            }
        } else {
            p++;
        }
        
        // 跳到行尾
        while (*p && *p != '\n') p++;
        if (*p == '\n') {
            line++;
            p++;
        }
    }
    
    printf("\n3. 符号表总览:\n");
    printf("  总符号数: %d\n", symbol_count);
    for (int i = 0; i < symbol_count; i++) {
        printf("  [%d] %s: %s (行%d, 作用域%d)\n", 
               i + 1, symbol_table[i].name, symbol_table[i].type, 
               symbol_table[i].line, symbol_table[i].scope_level);
    }

    printf("\n=== 语义分析完成 ===\n\n\n");

    // 清理资源
    vm_close(vm);
    run(input);
}