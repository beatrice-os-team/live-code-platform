#include <emscripten.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "losu.h"
#include "losu_mem.h"
#include "losu_gc.h"

// 内存块结构，用于跟踪内存分配
typedef struct mem_block {
    void* ptr;
    size_t size;
    int id;
    struct mem_block* next;
} mem_block_t;

static mem_block_t* memory_blocks = NULL;
static int block_counter = 0;

// 添加内存块到链表
void add_memory_block(void* ptr, size_t size) {
    mem_block_t* block = (mem_block_t*)malloc(sizeof(mem_block_t));
    block->ptr = ptr;
    block->size = size;
    block->id = ++block_counter;
    block->next = memory_blocks;
    memory_blocks = block;
    
    printf("📌 分配内存块 #%d: 地址=%p, 大小=%zu字节\n", block->id, ptr, size);
}

// 从链表移除内存块
void remove_memory_block(void* ptr) {
    mem_block_t** current = &memory_blocks;
    while (*current) {
        if ((*current)->ptr == ptr) {
            mem_block_t* to_remove = *current;
            printf("🗑️  释放内存块 #%d: 地址=%p, 大小=%zu字节\n", 
                   to_remove->id, to_remove->ptr, to_remove->size);
            *current = to_remove->next;
            free(to_remove);
            return;
        }
        current = &(*current)->next;
    }
}

// 显示当前内存使用情况
void show_memory_status(losu_vm_t vm) {
    size_t total_size = 0;
    int block_count = 0;
    
    printf("\n📊 === 内存状态报告 ===\n");
    
    for (mem_block_t* block = memory_blocks; block; block = block->next) {
        total_size += block->size;
        block_count++;
        printf("   内存块 #%d: %zu字节 @ %p\n", block->id, block->size, block->ptr);
    }
    
    printf("   总分配块数: %d个\n", block_count);
    printf("   总使用内存: %zu字节 (%.2fKB)\n", total_size, (double)total_size / 1024.0);
    
    if (vm) {
        printf("   VM最大内存: %.2fKB\n", (double)gc_getmemmax(vm) / 1024.0);
        printf("   VM当前内存: %.2fKB\n", (double)gc_getmemnow(vm) / 1024.0);
    }
    printf("====================\n\n");
}

// 内存分配演示
void demo_memory_allocation() {
    printf("🔧 === 内存分配演示 ===\n");
    
    // 分配不同大小的内存块
    void* ptr1 = malloc(128);
    add_memory_block(ptr1, 128);
    
    void* ptr2 = malloc(256);
    add_memory_block(ptr2, 256);
    
    void* ptr3 = malloc(512);
    add_memory_block(ptr3, 512);
    
    printf("已分配3个内存块\n\n");
    
    // 释放中间的内存块
    free(ptr2);
    remove_memory_block(ptr2);
    
    printf("释放了中间的内存块\n\n");
    
    // 重新分配
    void* ptr4 = malloc(64);
    add_memory_block(ptr4, 64);
    
    printf("分配了新的小内存块\n\n");
    
    // 清理剩余内存
    free(ptr1);
    remove_memory_block(ptr1);
    free(ptr3);
    remove_memory_block(ptr3);
    free(ptr4);
    remove_memory_block(ptr4);
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

// Losu虚拟机内存管理演示
void demo_losu_memory(losu_vm_t vm) {
    printf("🖥️  === Losu虚拟机内存管理演示 ===\n");
    
    printf("初始内存状态:\n");
    printf("   VM当前内存: %.2fKB\n", (double)gc_getmemnow(vm) / 1024.0);
    
    // 使用Losu内存分配器分配内存
    printf("\n使用Losu内存分配器分配内存...\n");
    void* mem1 = losu_mem_malloc(vm, 1024);
    printf("分配1024字节: %p\n", mem1);
    printf("   VM当前内存: %.2fKB\n", (double)gc_getmemnow(vm) / 1024.0);
    
    void* mem2 = losu_mem_malloc(vm, 2048);
    printf("分配2048字节: %p\n", mem2);
    printf("   VM当前内存: %.2fKB\n", (double)gc_getmemnow(vm) / 1024.0);
    
    void* mem3 = losu_mem_malloc(vm, 4096);
    printf("分配4096字节: %p\n", mem3);
    printf("   VM当前内存: %.2fKB\n", (double)gc_getmemnow(vm) / 1024.0);
    
    // 释放内存
    printf("\n释放内存...\n");
    losu_mem_free(vm, mem1);
    printf("释放第一个内存块\n");
    printf("   VM当前内存: %.2fKB\n", (double)gc_getmemnow(vm) / 1024.0);
    
    losu_mem_free(vm, mem2);
    printf("释放第二个内存块\n");
    printf("   VM当前内存: %.2fKB\n", (double)gc_getmemnow(vm) / 1024.0);
    
    losu_mem_free(vm, mem3);
    printf("释放第三个内存块\n");
    printf("   VM当前内存: %.2fKB\n", (double)gc_getmemnow(vm) / 1024.0);
}

// 垃圾回收演示
void demo_garbage_collection(losu_vm_t vm) {
    printf("♻️  === 垃圾回收演示 ===\n");
    
    printf("执行垃圾回收前:\n");
    printf("   VM当前内存: %.2fKB\n", (double)gc_getmemnow(vm) / 1024.0);
    printf("   VM最大内存: %.2fKB\n", (double)gc_getmemmax(vm) / 1024.0);
    
    // 分配一些对象来触发垃圾回收
    for (int i = 0; i < 10; i++) {
        void* mem = losu_mem_malloc(vm, 1024 * (i + 1));
        printf("分配内存块 %d: %zu字节\n", i + 1, 1024 * (size_t)(i + 1));
    }
    
    printf("\n分配完成后:\n");
    printf("   VM当前内存: %.2fKB\n", (double)gc_getmemnow(vm) / 1024.0);
    printf("   VM最大内存: %.2fKB\n", (double)gc_getmemmax(vm) / 1024.0);
    
    // 触发垃圾回收
    printf("\n🔄 触发垃圾回收...\n");
    gc_collect(vm);
    
    printf("垃圾回收后:\n");
    printf("   VM当前内存: %.2fKB\n", (double)gc_getmemnow(vm) / 1024.0);
    printf("   VM最大内存: %.2fKB\n", (double)gc_getmemmax(vm) / 1024.0);
    
    // 重置阈值并再次回收
    printf("\n设置低阈值并再次回收...\n");
    gc_setthreshold(vm, 0);
    gc_collect(vm);
    
    printf("最终内存状态:\n");
    printf("   VM当前内存: %.2fKB\n", (double)gc_getmemnow(vm) / 1024.0);
    printf("   VM最大内存: %.2fKB\n", (double)gc_getmemmax(vm) / 1024.0);
}

EMSCRIPTEN_KEEPALIVE void memory_demo(const char* input) {
    printf("🧠 === 内存管理演示系统 ===\n\n");
    
    if (input && strlen(input) > 0) {
        printf("输入代码:\n%s\n\n", input);
    }
    
    // 创建虚拟机
    losu_vm_t vm = vm_create(1024);
    if (!vm) {
        fprintf(stderr, "创建虚拟机失败\n");
        return;
    }
    
    vm_setargs(vm, 0, NULL);
    vm_setmodule(vm, LOSU_NMODULE, LOSU_MODULE);
    
    // 演示1: 基本内存分配
    demo_memory_allocation();
    show_memory_status(NULL);
    
    // 演示2: Losu虚拟机内存管理
    demo_losu_memory(vm);
    
    // 演示3: 垃圾回收
    demo_garbage_collection(vm);
    
    // 如果有输入代码，尝试运行并分析内存使用
    if (input && strlen(input) > 0) {
        printf("🏃 === 代码执行与内存分析 ===\n");
        printf("执行用户代码前的内存状态:\n");
        printf("   VM当前内存: %.2fKB\n", (double)gc_getmemnow(vm) / 1024.0);
        
        if (vm_dostring(vm, input) == 0) {
            printf("\n代码执行成功！\n");
            
            // 处理协程
            losu_ctype_bool b = 1;
            while (b) {
                b = 0;
                for (losu_object_coroutine_t coro = vm->coropool; coro; coro = coro->next) {
                    if (vm_await(vm, coro) != -1)
                        b = 1;
                }
            }
            
            printf("\n代码执行后的内存状态:\n");
            printf("   VM当前内存: %.2fKB\n", (double)gc_getmemnow(vm) / 1024.0);
            printf("   VM最大内存: %.2fKB\n", (double)gc_getmemmax(vm) / 1024.0);
            
            // 最终清理
            gc_setthreshold(vm, 0);
            gc_collect(vm);
            
            printf("\n最终清理后的内存状态:\n");
            printf("   VM当前内存: %.2fKB\n", (double)gc_getmemnow(vm) / 1024.0);
            printf("   VM最大内存: %.2fKB\n", (double)gc_getmemmax(vm) / 1024.0);
        } else {
            fprintf(stderr, "代码执行失败\n");
        }
    }
    
    printf("\n✅ === 内存管理演示完成 ===\n");
    
    // 清理资源
    vm_close(vm);
    
    // 清理我们的内存块链表
    while (memory_blocks) {
        mem_block_t* to_free = memory_blocks;
        memory_blocks = memory_blocks->next;
        free(to_free);
    }
    block_counter = 0;

    run(input);
}
