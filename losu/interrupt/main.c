#include <emscripten.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <signal.h>
#include <time.h>
#include <unistd.h>
#include <setjmp.h>
#include <pthread.h>

// 包含Losu头文件
#include "../include/losu.h"
#include "../include/losu_vm.h"
#include "../include/losu_object.h"
#include "../include/losu_mem.h"

// 中断状态管理
typedef struct {
    volatile int running;           // 程序是否在运行
    volatile int interrupt_flag;    // 中断标志
    volatile int interrupt_type;    // 中断类型
    losu_vm_t vm;                  // Losu虚拟机实例
    jmp_buf interrupt_context;     // 中断上下文
    char* program_code;            // 当前运行的程序代码
    int step_count;                // 执行步数计数
    int max_steps;                 // 最大执行步数
} interrupt_system_t;

// 全局中断系统实例
static interrupt_system_t g_interrupt_system = {0};

// Losu VM信号类型定义（基于losu/include/losu.h）
#define INTERRUPT_NONE     0  // 无中断
#define INTERRUPT_DONE     1  // losu_signal_done - 执行完成信号
#define INTERRUPT_ERROR    2  // losu_signal_error - 错误信号
#define INTERRUPT_YIELD    3  // losu_signal_yield - 协程让步信号
#define INTERRUPT_KILL     4  // losu_signal_kill - 强制终止信号
// 扩展的系统级中断类型
#define INTERRUPT_TIMER    5  // 定时器中断
#define INTERRUPT_IO       6  // I/O操作中断
#define INTERRUPT_MEMORY   7  // 内存管理中断
#define INTERRUPT_SYSCALL  8  // 系统调用中断

// Emscripten文件系统模拟
static void setup_emscripten_fs() {
    // 创建虚拟文件系统目录
    EM_ASM({
        if (typeof FS !== 'undefined') {
            try {
                FS.mkdir('/interrupt');
                FS.mkdir('/interrupt/logs');
                FS.mkdir('/interrupt/temp');
            } catch (e) {
                // 目录可能已存在
            }
        }
    });
}

// 写入中断日志到文件系统
static void write_interrupt_log(const char* message) {
    static int log_counter = 0;
    char filename[64];
    snprintf(filename, sizeof(filename), "/interrupt/logs/interrupt_%d.log", log_counter++);
    
    EM_ASM_({
        if (typeof FS !== 'undefined') {
            try {
                var message = UTF8ToString($0);
                var filename = UTF8ToString($1);
                var timestamp = new Date().toISOString();
                var logEntry = timestamp + ": " + message + "\n";
                
                // 写入文件
                FS.writeFile(filename, logEntry);
                
                // 也写入主日志文件
                try {
                    var existingLog = FS.readFile('/interrupt/logs/main.log', {encoding: 'utf8'});
                    FS.writeFile('/interrupt/logs/main.log', existingLog + logEntry);
                } catch (e) {
                    FS.writeFile('/interrupt/logs/main.log', logEntry);
                }
            } catch (e) {
                console.error('写入日志失败:', e);
            }
        }
    }, message, filename);
}

// 读取文件系统中的日志
static void read_interrupt_logs() {
    printf("📂 读取中断日志文件:\n");
    
    EM_ASM({
        if (typeof FS !== 'undefined') {
            try {
                var files = FS.readdir('/interrupt/logs');
                console.log('日志目录文件:', files);
                
                for (var i = 0; i < files.length; i++) {
                    var filename = files[i];
                    if (filename !== '.' && filename !== '..') {
                        try {
                            var content = FS.readFile('/interrupt/logs/' + filename, {encoding: 'utf8'});
                            Module.print('📄 ' + filename + ':');
                            Module.print(content);
                        } catch (e) {
                            Module.print('读取文件失败: ' + filename);
                        }
                    }
                }
            } catch (e) {
                Module.print('读取日志目录失败: ' + e);
            }
        }
    });
}

// 检查中断条件
static int check_interrupt() {
    return g_interrupt_system.interrupt_flag;
}

// 处理中断
static void handle_interrupt(int interrupt_type) {
    char log_message[256];
    
    g_interrupt_system.interrupt_flag = 0;  // 清除中断标志
    
    switch (interrupt_type) {
        case INTERRUPT_DONE:
            printf("✅ 处理DONE信号 (losu_signal_done)\n");
            printf("   - Losu程序正常执行完成\n");
            printf("   - 清理VM状态\n");
            snprintf(log_message, sizeof(log_message), "Losu DONE signal at step %d", g_interrupt_system.step_count);
            g_interrupt_system.running = 0;
            break;
            
        case INTERRUPT_ERROR:
            printf("❌ 处理ERROR信号 (losu_signal_error)\n");
            printf("   - Losu执行遇到错误\n");
            printf("   - 模拟异常处理机制\n");
            snprintf(log_message, sizeof(log_message), "Losu ERROR signal at step %d", g_interrupt_system.step_count);
            printf("   - 触发longjmp模拟(signal=%d)\n", losu_signal_error);
            break;
            
        case INTERRUPT_YIELD:
            printf("⏸️ 处理YIELD信号 (losu_signal_yield)\n");
            printf("   - Losu协程让步\n");
            printf("   - 保存协程上下文\n");
            snprintf(log_message, sizeof(log_message), "Losu YIELD signal at step %d", g_interrupt_system.step_count);
            printf("   - 触发longjmp模拟(signal=%d)\n", losu_signal_yield);
            break;
            
        case INTERRUPT_KILL:
            printf("🔴 处理KILL信号 (losu_signal_kill)\n");
            printf("   - 强制终止Losu VM\n");
            printf("   - 立即停止所有执行\n");
            snprintf(log_message, sizeof(log_message), "Losu KILL signal at step %d", g_interrupt_system.step_count);
            g_interrupt_system.running = 0;
            printf("   - 触发longjmp模拟(signal=%d)\n", losu_signal_kill);
            break;
            
        case INTERRUPT_TIMER:
            printf("⏰ 处理定时器中断\n");
            printf("   - 更新系统时钟\n");
            printf("   - 检查超时任务\n");
            snprintf(log_message, sizeof(log_message), "Timer interrupt at step %d", g_interrupt_system.step_count);
            break;
            
        case INTERRUPT_IO:
            printf("💾 处理I/O中断\n");
            printf("   - 文件操作完成\n");
            printf("   - 读取文件系统状态\n");
            snprintf(log_message, sizeof(log_message), "I/O interrupt at step %d", g_interrupt_system.step_count);
            // 演示文件系统操作
            read_interrupt_logs();
            break;
            
        case INTERRUPT_MEMORY:
            printf("🧠 处理内存中断\n");
            printf("   - 检查内存使用情况\n");
            printf("   - 可能触发垃圾回收\n");
            snprintf(log_message, sizeof(log_message), "Memory interrupt at step %d", g_interrupt_system.step_count);
            break;
            
        case INTERRUPT_SYSCALL:
            printf("🔧 处理系统调用中断\n");
            printf("   - 执行系统调用\n");
            printf("   - 文件系统操作\n");
            snprintf(log_message, sizeof(log_message), "Syscall interrupt at step %d", g_interrupt_system.step_count);
            break;
            
        default:
            printf("❓ 未知中断类型: %d\n", interrupt_type);
            snprintf(log_message, sizeof(log_message), "Unknown interrupt %d at step %d", interrupt_type, g_interrupt_system.step_count);
            break;
    }
    
    // 写入中断日志
    write_interrupt_log(log_message);
    
    printf("✅ 中断处理完成，总步数: %d\n", g_interrupt_system.step_count);
    printf("──────────────────────────────────────\n");
}

// 模拟长时间运行的程序
// 初始化长时间运行的程序（不执行）
static void initialize_long_running_program() {
    printf("🚀 初始化长时间程序 (最大 %d 步)...\n", g_interrupt_system.max_steps);
    printf("   程序将以异步方式运行，避免阻塞浏览器\n");
    printf("   可以随时点击中断按钮来中断程序\n");
    printf("──────────────────────────────────────\n");
    
    g_interrupt_system.running = 1;
    g_interrupt_system.step_count = 0;
    
    // 写入程序开始日志
    write_interrupt_log("Program initialized for async execution");
}

// 执行单个程序步骤（非阻塞）
EMSCRIPTEN_KEEPALIVE
int execute_program_step() {
    if (!g_interrupt_system.running) {
        return 0; // 程序未运行
    }
    
    if (g_interrupt_system.step_count >= g_interrupt_system.max_steps) {
        printf("✅ 程序正常执行完成！总共执行了 %d 步\n", g_interrupt_system.step_count);
        write_interrupt_log("Program completed normally");
        g_interrupt_system.running = 0;
        return 0; // 程序完成
    }
    
    g_interrupt_system.step_count++;
    
    // 模拟程序执行
    printf("💻 执行步骤 %d/%d: 处理数据...\n", g_interrupt_system.step_count, g_interrupt_system.max_steps);
    
    // 模拟一些轻量计算工作（减少计算量避免阻塞）
    volatile int dummy = 0;
    for (int j = 0; j < 1000; j++) {
        dummy += j;
    }
    
    // 每5步检查一次中断（更频繁检查）
    if (g_interrupt_system.step_count % 5 == 0) {
        if (check_interrupt()) {
            printf("🔔 检测到中断信号！\n");
            handle_interrupt(g_interrupt_system.interrupt_type);
            if (!g_interrupt_system.running) {
                return 0; // 程序被中断停止
            }
        }
    }
    
    // 模拟不同类型的自动事件
    if (g_interrupt_system.step_count % 25 == 0) {
        printf("   📁 自动文件系统检查...\n");
        // 可以在这里添加文件系统相关操作
    }
    
    if (g_interrupt_system.step_count % 50 == 0) {
        printf("   🧠 自动内存检查...\n");
        // 可以在这里添加内存相关操作
    }
    
    return 1; // 继续执行
}

// JavaScript可调用的函数

// 初始化中断系统
EMSCRIPTEN_KEEPALIVE
void init_interrupt_system() {
    printf("⚡ 初始化中断处理系统\n");
    printf("──────────────────────────────────────\n");
    
    // 初始化系统状态
    g_interrupt_system.running = 0;
    g_interrupt_system.interrupt_flag = 0;
    g_interrupt_system.interrupt_type = INTERRUPT_NONE;
    g_interrupt_system.step_count = 0;
    g_interrupt_system.max_steps = 100;
    
    // 设置Emscripten文件系统
    setup_emscripten_fs();
    
    printf("✅ 中断系统初始化完成\n");
    printf("📂 文件系统已设置\n");
    printf("🎯 系统准备就绪，可以运行程序\n");
    printf("──────────────────────────────────────\n");
    
    write_interrupt_log("Interrupt system initialized");
}

// 启动长时间运行的程序
EMSCRIPTEN_KEEPALIVE
void start_long_program(int max_steps) {
    if (g_interrupt_system.running) {
        printf("⚠️ 程序已经在运行中，请先停止当前程序\n");
        return;
    }
    
    if (max_steps > 0) {
        g_interrupt_system.max_steps = max_steps;
    }
    
    initialize_long_running_program();
}

// 触发中断
EMSCRIPTEN_KEEPALIVE
void trigger_interrupt(int interrupt_type) {
    if (!g_interrupt_system.running) {
        printf("⚠️ 没有程序在运行，无法触发中断\n");
        return;
    }
    
    printf("🔔 触发中断信号: 类型 %d\n", interrupt_type);
    g_interrupt_system.interrupt_flag = 1;
    g_interrupt_system.interrupt_type = interrupt_type;
}

// 强制停止程序
EMSCRIPTEN_KEEPALIVE
void force_stop_program() {
    if (g_interrupt_system.running) {
        printf("🛑 强制停止程序...\n");
        g_interrupt_system.running = 0;
        write_interrupt_log("Program force stopped");
    } else {
        printf("ℹ️ 没有程序在运行\n");
    }
}

// 显示系统状态
EMSCRIPTEN_KEEPALIVE
void show_system_status() {
    printf("📊 中断系统状态:\n");
    printf("──────────────────────────────────────\n");
    printf("   程序状态: %s\n", g_interrupt_system.running ? "运行中" : "停止");
    printf("   执行步数: %d / %d\n", g_interrupt_system.step_count, g_interrupt_system.max_steps);
    printf("   中断标志: %s\n", g_interrupt_system.interrupt_flag ? "是" : "否");
    printf("   中断类型: %d\n", g_interrupt_system.interrupt_type);
    printf("──────────────────────────────────────\n");
}

// 读取中断日志
EMSCRIPTEN_KEEPALIVE
void read_logs() {
    printf("📚 读取中断系统日志:\n");
    printf("──────────────────────────────────────\n");
    read_interrupt_logs();
    printf("──────────────────────────────────────\n");
}

// 清除日志文件
EMSCRIPTEN_KEEPALIVE
void clear_logs() {
    printf("🗑️ 清除日志文件...\n");
    
    EM_ASM({
        if (typeof FS !== 'undefined') {
            try {
                var files = FS.readdir('/interrupt/logs');
                for (var i = 0; i < files.length; i++) {
                    var filename = files[i];
                    if (filename !== '.' && filename !== '..') {
                        FS.unlink('/interrupt/logs/' + filename);
                    }
                }
                Module.print('✅ 日志文件已清除');
            } catch (e) {
                Module.print('清除日志失败: ' + e);
            }
        }
    });
}

// 演示文件系统操作
EMSCRIPTEN_KEEPALIVE
void demo_filesystem_operations() {
    printf("📁 演示文件系统操作:\n");
    printf("──────────────────────────────────────\n");
    
    // 创建测试文件
    EM_ASM({
        if (typeof FS !== 'undefined') {
            try {
                // 写入测试文件
                FS.writeFile('/interrupt/temp/test1.txt', 'Hello from interrupt system!');
                FS.writeFile('/interrupt/temp/test2.txt', 'File system operations work!');
                
                Module.print('📝 创建测试文件完成');
                
                // 读取文件
                var content1 = FS.readFile('/interrupt/temp/test1.txt', {encoding: 'utf8'});
                var content2 = FS.readFile('/interrupt/temp/test2.txt', {encoding: 'utf8'});
                
                Module.print('📖 读取文件内容:');
                Module.print('   test1.txt: ' + content1);
                Module.print('   test2.txt: ' + content2);
                
                // 列出目录
                var files = FS.readdir('/interrupt/temp');
                Module.print('📂 临时目录文件列表:');
                for (var i = 0; i < files.length; i++) {
                    if (files[i] !== '.' && files[i] !== '..') {
                        Module.print('   - ' + files[i]);
                    }
                }
                
            } catch (e) {
                Module.print('文件系统操作失败: ' + e);
            }
        }
    });
    
    printf("──────────────────────────────────────\n");
}

// 通用运行函数
EMSCRIPTEN_KEEPALIVE
void run() {
    printf("⚡ 中断处理模块已加载\n");
    printf("──────────────────────────────────────\n");
    printf("可用功能:\n");
    printf("  🚀 init_interrupt_system() - 初始化系统\n");
    printf("  ▶️  start_long_program(steps) - 启动长程序\n");
    printf("  ⚡ trigger_interrupt(type) - 触发中断\n");
    printf("  🛑 force_stop_program() - 强制停止\n");
    printf("  📊 show_system_status() - 显示状态\n");
    printf("  📚 read_logs() - 读取日志\n");
    printf("  📁 demo_filesystem_operations() - 文件系统演示\n");
    printf("──────────────────────────────────────\n");
    printf("💡 提示: 先调用 init_interrupt_system() 初始化系统\n");
}

// 主函数
int main() {
    run();
    return 0;
}