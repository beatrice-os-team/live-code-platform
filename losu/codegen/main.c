#include <emscripten.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#include "losu.h"

// 虚拟指令集
typedef enum {
    OP_LOAD,    // 加载值
    OP_STORE,   // 存储值
    OP_ADD,     // 加法
    OP_SUB,     // 减法
    OP_MUL,     // 乘法
    OP_DIV,     // 除法
    OP_JMP,     // 无条件跳转
    OP_JZ,      // 条件跳转（为零时跳转）
    OP_CALL,    // 函数调用
    OP_RET,     // 返回
    OP_PRINT,   // 打印
    OP_HALT     // 停机
} OpCode;

typedef struct {
    OpCode op;
    int arg1;
    int arg2;
    char comment[64];
} Instruction;

Instruction code[200];
int code_size = 0;

const char* op_names[] = {
    "LOAD", "STORE", "ADD", "SUB", "MUL", "DIV",
    "JMP", "JZ", "CALL", "RET", "PRINT", "HALT"
};

void emit(OpCode op, int arg1, int arg2, const char* comment) {
    if (code_size < 200) {
        code[code_size].op = op;
        code[code_size].arg1 = arg1;
        code[code_size].arg2 = arg2;
        strncpy(code[code_size].comment, comment, 63);
        code_size++;
    }
}

EMSCRIPTEN_KEEPALIVE void codegen_demo(const char* input) {
    if (!input || strlen(input) == 0) {
        printf("代码生成输入为空\n");
        return;
    }

    printf("=== 代码生成演示 ===\n");
    printf("输入代码:\n%s\n", input);
    printf("\n=== 字节码生成过程 ===\n");
    
    // 创建虚拟机
    losu_vm_t vm = vm_create(1024);
    if (!vm) {
        fprintf(stderr, "创建虚拟机失败\n");
        return;
    }

    vm_setargs(vm, 0, NULL);
    vm_setmodule(vm, LOSU_NMODULE, LOSU_MODULE);

    // 重置代码生成器
    code_size = 0;

    printf("开始代码生成...\n");
    printf("\n1. 代码分析与指令生成:\n");
    
    // 简单的代码生成
    const char* p = input;
    int line = 1;
    int var_counter = 0;
    
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
            printf("  处理函数定义 (行%d)\n", line);
            p += 4;
            while (*p && *p == ' ') p++;
            
            char func_name[64];
            int i = 0;
            while (*p && *p != '(' && *p != ' ' && i < 63) {
                func_name[i++] = *p++;
            }
            func_name[i] = '\0';
            
            char comment[64];
            snprintf(comment, 64, "Function: %s", func_name);
            emit(OP_CALL, 0, 0, comment);
            printf("    生成指令: CALL (函数: %s)\n", func_name);
        }
        // 变量声明与赋值
        else if (strncmp(p, "let ", 4) == 0) {
            printf("  处理变量声明 (行%d)\n", line);
            p += 4;
            while (*p && *p == ' ') p++;
            
            char var_name[64];
            int i = 0;
            while (*p && *p != '=' && *p != ' ' && *p != '\n' && i < 63) {
                var_name[i++] = *p++;
            }
            var_name[i] = '\0';
            
            // 跳过等号
            while (*p && (*p == ' ' || *p == '=')) p++;
            
            // 解析赋值表达式
            if (isdigit(*p)) {
                int value = 0;
                while (*p && isdigit(*p)) {
                    value = value * 10 + (*p - '0');
                    p++;
                }
                
                char comment[64];
                snprintf(comment, 64, "Load %d -> %s", value, var_name);
                emit(OP_LOAD, value, 0, comment);
                printf("    生成指令: LOAD %d (值: %d)\n", var_counter, value);
                
                snprintf(comment, 64, "Store var[%d] (%s)", var_counter, var_name);
                emit(OP_STORE, var_counter, 0, comment);
                printf("    生成指令: STORE %d (变量: %s)\n", var_counter, var_name);
                var_counter++;
            }
        }
        // 表达式语句
        else if (isdigit(*p) || isalpha(*p)) {
            char expr[128];
            int i = 0;
            const char* expr_start = p;
            
            while (*p && *p != '\n' && i < 127) {
                expr[i++] = *p++;
            }
            expr[i] = '\0';
            
            if (strchr(expr, '+')) {
                printf("  处理加法表达式 (行%d): %s\n", line, expr);
                emit(OP_ADD, 0, 1, "Addition operation");
                printf("    生成指令: ADD 0, 1\n");
            } else if (strchr(expr, '-')) {
                printf("  处理减法表达式 (行%d): %s\n", line, expr);
                emit(OP_SUB, 0, 1, "Subtraction operation");
                printf("    生成指令: SUB 0, 1\n");
            } else if (strchr(expr, '*')) {
                printf("  处理乘法表达式 (行%d): %s\n", line, expr);
                emit(OP_MUL, 0, 1, "Multiplication operation");
                printf("    生成指令: MUL 0, 1\n");
            } else if (strchr(expr, '/')) {
                printf("  处理除法表达式 (行%d): %s\n", line, expr);
                emit(OP_DIV, 0, 1, "Division operation");
                printf("    生成指令: DIV 0, 1\n");
            } else if (strstr(expr, "print")) {
                printf("  处理打印语句 (行%d): %s\n", line, expr);
                emit(OP_PRINT, 0, 0, "Print statement");
                printf("    生成指令: PRINT\n");
            }
        }
        
        // 跳到行尾
        while (*p && *p != '\n') p++;
        if (*p == '\n') {
            line++;
            p++;
        }
    }
    
    // 添加结束指令
    emit(OP_HALT, 0, 0, "Program end");
    printf("    生成指令: HALT (程序结束)\n");
    
    printf("\n2. 生成的字节码:\n");
    printf("地址  指令      参数1  参数2  注释\n");
    printf("----  --------  -----  -----  ----\n");
    
    for (int i = 0; i < code_size; i++) {
        printf("%04d  %-8s  %5d  %5d  %s\n", 
               i, op_names[code[i].op], code[i].arg1, code[i].arg2, code[i].comment);
    }
    
    printf("\n3. 代码优化分析:\n");
    int loads = 0, stores = 0, ops = 0;
    for (int i = 0; i < code_size; i++) {
        if (code[i].op == OP_LOAD) loads++;
        else if (code[i].op == OP_STORE) stores++;
        else if (code[i].op >= OP_ADD && code[i].op <= OP_DIV) ops++;
    }
    
    printf("  - 加载指令: %d 条\n", loads);
    printf("  - 存储指令: %d 条\n", stores);
    printf("  - 运算指令: %d 条\n", ops);
    printf("  - 总指令数: %d 条\n", code_size);
    
    if (loads > stores * 2) {
        printf("  💡 优化建议: 可能存在冗余的加载操作\n");
    }
    if (code_size > 50) {
        printf("  💡 优化建议: 代码较长，可考虑函数拆分\n");
    }

    printf("\n=== 代码生成完成 ===\n");

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