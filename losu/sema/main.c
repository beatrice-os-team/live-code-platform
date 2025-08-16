#include <emscripten.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#include "losu.h"

// 符号类型枚举
typedef enum {
    SYMBOL_GLOBAL_VAR,
    SYMBOL_GLOBAL_FUNC,
    SYMBOL_CLASS,
    SYMBOL_MEMBER_FUNC,
    SYMBOL_MEMBER_VAR,
    SYMBOL_LOCAL_VAR
} SymbolType;

// 符号表结构
typedef struct {
    char name[64];
    SymbolType type;
    int line;
    int scope_level;
    int indent_level;
    char class_name[64];  // 对于成员函数和成员变量，记录所属类名
} Symbol;

Symbol symbol_table[200];
int symbol_count = 0;
int current_scope = 0;
int current_indent = 0;
char current_class[64] = "";

// 缩进栈，用于跟踪作用域
typedef struct {
    int indent_level;
    char scope_type[32];  // "class", "function", "global"
    char scope_name[64];
} IndentStack;

IndentStack indent_stack[50];
int indent_stack_top = 0;

void push_indent_stack(int indent, const char* type, const char* name) {
    if (indent_stack_top < 50) {
        indent_stack[indent_stack_top].indent_level = indent;
        strncpy(indent_stack[indent_stack_top].scope_type, type, 31);
        strncpy(indent_stack[indent_stack_top].scope_name, name, 63);
        indent_stack_top++;
    }
}

void pop_indent_stack(int indent) {
    while (indent_stack_top > 0 && indent_stack[indent_stack_top - 1].indent_level > indent) {
        indent_stack_top--;
    }
}

// 获取当前作用域信息
void get_current_scope_info(char* type, char* name) {
    if (indent_stack_top > 0) {
        strcpy(type, indent_stack[indent_stack_top - 1].scope_type);
        strcpy(name, indent_stack[indent_stack_top - 1].scope_name);
    } else {
        strcpy(type, "global");
        strcpy(name, "");
    }
}

void add_symbol(const char* name, SymbolType type, int line, int indent) {
    if (symbol_count < 200) {
        strncpy(symbol_table[symbol_count].name, name, 63);
        symbol_table[symbol_count].type = type;
        symbol_table[symbol_count].line = line;
        symbol_table[symbol_count].scope_level = current_scope;
        symbol_table[symbol_count].indent_level = indent;
        
        // 如果是成员函数或成员变量，记录所属类名
        if (type == SYMBOL_MEMBER_FUNC || type == SYMBOL_MEMBER_VAR) {
            strncpy(symbol_table[symbol_count].class_name, current_class, 63);
        } else {
            symbol_table[symbol_count].class_name[0] = '\0';
        }
        
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

// 计算行的缩进级别
int get_indent_level(const char* line) {
    int indent = 0;
    while (*line && (*line == ' ' || *line == '\t')) {
        if (*line == ' ') {
            indent++;
        } else {
            indent += 4;  // 假设tab等于4个空格
        }
        line++;
    }
    return indent;
}

// 跳过前导空格
const char* skip_indent(const char* line) {
    while (*line && (*line == ' ' || *line == '\t')) {
        line++;
    }
    return line;
}

// 检查是否是关键字
int is_keyword(const char* word) {
    const char* keywords[] = {
        "def", "let", "if", "else", "while", "for", "return", 
        "class", "self", "async", "yield", "lambda", "true", "false"
    };
    int num_keywords = sizeof(keywords) / sizeof(keywords[0]);
    
    for (int i = 0; i < num_keywords; i++) {
        if (strcmp(word, keywords[i]) == 0) {
            return 1;
        }
    }
    return 0;
}

// 获取符号类型字符串
const char* get_symbol_type_string(SymbolType type) {
    switch (type) {
        case SYMBOL_GLOBAL_VAR: return "全局变量";
        case SYMBOL_GLOBAL_FUNC: return "全局函数";
        case SYMBOL_CLASS: return "类";
        case SYMBOL_MEMBER_FUNC: return "成员函数";
        case SYMBOL_MEMBER_VAR: return "成员变量";
        case SYMBOL_LOCAL_VAR: return "局部变量";
        default: return "未知";
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

    // 重置符号表和状态
    symbol_count = 0;
    current_scope = 0;
    current_indent = 0;
    current_class[0] = '\0';
    indent_stack_top = 0;

    printf("开始语义分析...\n");
    printf("\n1. 符号表构建:\n");
    
    // 按行分析代码
    char* input_copy = strdup(input);
    char* line = strtok(input_copy, "\n");
    int line_num = 1;
    
    while (line) {
        int indent = get_indent_level(line);
        const char* content = skip_indent(line);
        
        // 跳过空行和注释
        if (strlen(content) == 0 || content[0] == '#') {
            line = strtok(NULL, "\n");
            line_num++;
            continue;
        }
        
        // 更新缩进栈和当前类名
        if (indent < current_indent) {
            pop_indent_stack(indent);
            // 更新当前类名
            current_class[0] = '\0';
            for (int i = 0; i < indent_stack_top; i++) {
                if (strcmp(indent_stack[i].scope_type, "class") == 0) {
                    strcpy(current_class, indent_stack[i].scope_name);
                    break;
                }
            }
        }
        current_indent = indent;
        
        // 如果缩进回到0，说明退出了所有作用域（包括类）
        if (indent == 0) {
            current_class[0] = '\0';
        }
        
        // 分析类定义
        if (strncmp(content, "class ", 6) == 0) {
            char class_name[64];
            int i = 0;
            const char* p = content + 6;
            while (*p && *p != ':' && *p != ' ' && i < 63) {
                class_name[i++] = *p++;
            }
            class_name[i] = '\0';
            
            if (strlen(class_name) > 0) {
                add_symbol(class_name, SYMBOL_CLASS, line_num, indent);
                printf("  添加类符号: %s (行%d, 缩进%d)\n", class_name, line_num, indent);
                push_indent_stack(indent, "class", class_name);
                strcpy(current_class, class_name);
            }
        }
        // 分析函数定义
        else if (strncmp(content, "def ", 4) == 0) {
            char func_name[64];
            int i = 0;
            const char* p = content + 4;
            while (*p && *p != '(' && *p != ' ' && i < 63) {
                func_name[i++] = *p++;
            }
            func_name[i] = '\0';
            
            if (strlen(func_name) > 0) {
                // 检查是否在类作用域内
                if (strlen(current_class) > 0) {
                    // 成员函数
                    add_symbol(func_name, SYMBOL_MEMBER_FUNC, line_num, indent);
                    printf("  添加成员函数: %s.%s (行%d, 缩进%d)\n", current_class, func_name, line_num, indent);
                } else {
                    // 全局函数
                    add_symbol(func_name, SYMBOL_GLOBAL_FUNC, line_num, indent);
                    printf("  添加全局函数: %s (行%d, 缩进%d)\n", func_name, line_num, indent);
                }
                push_indent_stack(indent, "function", func_name);
            }
        }
        // 分析变量声明
        else if (strncmp(content, "let ", 4) == 0) {
            const char* p = content + 4;
            while (*p && *p == ' ') p++;
            
            char var_name[64];
            int i = 0;
            const char* start = p;
            
            // 检查是否是self.变量
            if (strncmp(p, "self.", 5) == 0) {
                p += 5;
                while (*p && *p != '=' && *p != ' ' && *p != '\n' && i < 63) {
                    var_name[i++] = *p++;
                }
                var_name[i] = '\0';
                
                if (strlen(var_name) > 0 && strlen(current_class) > 0) {
                    add_symbol(var_name, SYMBOL_MEMBER_VAR, line_num, indent);
                    printf("  添加成员变量: %s.%s (行%d, 缩进%d)\n", current_class, var_name, line_num, indent);
                }
            } else {
                // 普通变量
                while (*p && *p != '=' && *p != ' ' && *p != '\n' && i < 63) {
                    var_name[i++] = *p++;
                }
                var_name[i] = '\0';
                
                if (strlen(var_name) > 0) {
                    // 检查是否在全局作用域（缩进为0）
                    if (indent == 0) {
                        add_symbol(var_name, SYMBOL_GLOBAL_VAR, line_num, indent);
                        printf("  添加全局变量: %s (行%d, 缩进%d)\n", var_name, line_num, indent);
                    } else {
                        // 在函数或类内部声明的变量都是局部变量
                        add_symbol(var_name, SYMBOL_LOCAL_VAR, line_num, indent);
                        printf("  添加局部变量: %s (行%d, 缩进%d)\n", var_name, line_num, indent);
                    }
                }
            }
        }
        
        line = strtok(NULL, "\n");
        line_num++;
    }
    
    free(input_copy);
    
    printf("\n2. 符号引用检查:\n");
    
    // 第二遍扫描：检查符号引用
    input_copy = strdup(input);
    line = strtok(input_copy, "\n");
    line_num = 1;
    
    while (line) {
        const char* content = skip_indent(line);
        
        // 跳过空行、注释和定义行
        if (strlen(content) == 0 || content[0] == '#' ||
            strncmp(content, "class ", 6) == 0 ||
            strncmp(content, "def ", 4) == 0 ||
            strncmp(content, "let ", 4) == 0) {
            line = strtok(NULL, "\n");
            line_num++;
            continue;
        }
        
        // 检查标识符引用
        const char* p = content;
        while (*p) {
            if (isalpha(*p) || *p == '_') {
                char identifier[64];
                int i = 0;
                const char* start = p;
                
                while (*p && (isalnum(*p) || *p == '_') && i < 63) {
                    identifier[i++] = *p++;
                }
                identifier[i] = '\0';
                
                // 跳过关键字
                if (!is_keyword(identifier) && strlen(identifier) > 0) {
                    Symbol* sym = find_symbol(identifier);
                    if (sym) {
                        const char* type_str = get_symbol_type_string(sym->type);
                        if (sym->type == SYMBOL_MEMBER_FUNC || sym->type == SYMBOL_MEMBER_VAR) {
                            printf("  ✓ 符号引用: %s.%s (定义在行%d, 类型: %s) 行: %d\n", 
                                   sym->class_name, identifier, sym->line, type_str, line_num);
                        } else {
                            printf("  ✓ 符号引用: %s (定义在行%d, 类型: %s) 行: %d\n", 
                                   identifier, sym->line, type_str, line_num);
                        }
                    }
                }
            } else {
                p++;
            }
        }
        
        line = strtok(NULL, "\n");
        line_num++;
    }
    
    free(input_copy);
    
    printf("\n3. 符号表总览:\n");
    printf("  总符号数: %d\n", symbol_count);
    for (int i = 0; i < symbol_count; i++) {
        const char* type_str = get_symbol_type_string(symbol_table[i].type);
        if (symbol_table[i].type == SYMBOL_MEMBER_FUNC || symbol_table[i].type == SYMBOL_MEMBER_VAR) {
            printf("  [%d] %s.%s: %s (行%d, 缩进%d)\n", 
                   i + 1, symbol_table[i].class_name, symbol_table[i].name, type_str, 
                   symbol_table[i].line, symbol_table[i].indent_level);
        } else {
            printf("  [%d] %s: %s (行%d, 缩进%d)\n", 
                   i + 1, symbol_table[i].name, type_str, 
                   symbol_table[i].line, symbol_table[i].indent_level);
        }
    }

    printf("\n4. 作用域层次结构:\n");
    for (int i = 0; i < indent_stack_top; i++) {
        printf("  作用域%d: %s '%s' (缩进%d)\n", 
               i + 1, indent_stack[i].scope_type, indent_stack[i].scope_name, 
               indent_stack[i].indent_level);
    }

    printf("\n=== 语义分析完成 ===\n\n\n");

    // 清理资源
    vm_close(vm);
    run(input);
}