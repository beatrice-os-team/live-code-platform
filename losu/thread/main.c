/*
  Losu Copyright Notice
  --------------------
    Losu is an open source programming language project under the MIT license
  that can be used for both academic and commercial purposes. There are no
  fees, no royalties, and no GNU-like restrictions. Losu qualifies
  as open source software. However, Losu is not public property, and founder
  'chen-chaochen' retains its copyright.

    Losu has been registered with the National Copyright Administration of the
  People's Republic of China, and adopts the MIT license as the copyright
  licensing contract under which the right holder conditionally licenses its
  reproduction, distribution, and modification rights to an unspecified public.

    If you use Losu, please follow the public MIT agreement or choose to enter
  into a dedicated license agreement with us.

  The MIT LICENSE is as follows
  --------------------
  Copyright  2020  chen-chaochen

    Permission is hereby granted, free of charge, to any person obtaining a
  copy of this software and associated documentation files (the "Software"), to
  deal in the Software without restriction, including without limitation the
  rights to use, copy, modify, merge, publish, distribute, sublicense, and/or
  sell copies of the Software, and to permit persons to whom the Software is
  furnished to do so, subject to the following conditions:

    The above copyright notice and this permission notice shall be included in
  all copies or substantial portions of the Software.

    THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
  IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
  FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
  AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
  LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
  OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
  SOFTWARE.
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>

// 为独立编译定义简化的类型
typedef struct thread_vm {
    int dummy;
} *thread_vm_t;

typedef struct thread_coroutine {
    int dummy;
} *thread_coroutine_t;

// 简化的协程创建函数（避免与Losu源码冲突）
thread_coroutine_t thread_create_coro(thread_vm_t vm, int size) {
    (void)vm; (void)size;
    return NULL;
}

#ifdef __EMSCRIPTEN__
#include <emscripten.h>
#endif

// 线程调度状态定义
typedef enum {
    THREAD_STATE_NEW = 0,
    THREAD_STATE_READY = 1,
    THREAD_STATE_RUNNING = 2,
    THREAD_STATE_BLOCKED = 3,
    THREAD_STATE_TERMINATED = 4
} thread_state_t;

// 调度算法类型
typedef enum {
    SCHEDULE_FCFS = 0,    // 先来先服务
    SCHEDULE_SJF = 1,     // 最短作业优先
    SCHEDULE_RR = 2,      // 时间片轮转
    SCHEDULE_PRIORITY = 3, // 优先级调度
    SCHEDULE_MLFQ = 4     // 多级反馈队列
} schedule_algorithm_t;

// 线程控制块
typedef struct thread_info {
    int thread_id;
    char name[64];
    thread_state_t state;
    int priority;
    int burst_time;
    int remaining_time;
    int arrival_time;
    int start_time;
    int completion_time;
    int waiting_time;
    int turnaround_time;
    int response_time;
    thread_coroutine_t coro;
    struct thread_info* next;
} thread_info_t;

// 调度器状态
typedef struct {
    thread_info_t* ready_queue;
    thread_info_t* running_thread;
    thread_info_t* blocked_queue;
    thread_info_t* terminated_queue;
    schedule_algorithm_t algorithm;
    int time_quantum;
    int current_time;
    int thread_counter;
    int context_switches;
    int current_time_slice;  // 添加当前线程已运行的时间片计数
    float avg_turnaround_time;
    float avg_waiting_time;
    float avg_response_time;
    float cpu_utilization;
} scheduler_t;

static scheduler_t g_scheduler = {0};

// 调度器输出函数
void thread_print_info(const char* format, ...) {
    char buffer[1024];
    va_list args;
    va_start(args, format);
    vsnprintf(buffer, sizeof(buffer), format, args);
    va_end(args);
    
    printf("[调度器] %s", buffer);
    fflush(stdout);
}

void thread_print_state_change(thread_info_t* thread, thread_state_t old_state, thread_state_t new_state) {
    const char* state_names[] = {"新建", "就绪", "运行", "阻塞", "终止"};
    thread_print_info("线程 %s (ID:%d) 状态变化: %s -> %s\n", 
        thread->name, thread->thread_id, 
        state_names[old_state], state_names[new_state]);
}

void thread_print_schedule_info(thread_info_t* thread) {
    thread_print_info("选择执行线程: %s (ID:%d, 优先级:%d, 剩余时间:%d)\n",
        thread->name, thread->thread_id, thread->priority, thread->remaining_time);
}

void thread_print_statistics() {
    thread_print_info("=== 调度统计信息 ===\n");
    thread_print_info("当前时间: %d\n", g_scheduler.current_time);
    thread_print_info("上下文切换次数: %d\n", g_scheduler.context_switches);
    thread_print_info("平均周转时间: %.2f\n", g_scheduler.avg_turnaround_time);
    thread_print_info("平均等待时间: %.2f\n", g_scheduler.avg_waiting_time);
    thread_print_info("平均响应时间: %.2f\n", g_scheduler.avg_response_time);
    thread_print_info("CPU利用率: %.2f%%\n", g_scheduler.cpu_utilization * 100);
}

// 线程队列操作
void enqueue_thread(thread_info_t** queue, thread_info_t* thread) {
    if (*queue == NULL) {
        *queue = thread;
        thread->next = NULL;
    } else {
        thread_info_t* current = *queue;
        while (current->next != NULL) {
            current = current->next;
        }
        current->next = thread;
        thread->next = NULL;
    }
}

thread_info_t* dequeue_thread(thread_info_t** queue) {
    if (*queue == NULL) return NULL;
    
    thread_info_t* thread = *queue;
    *queue = thread->next;
    thread->next = NULL;
    return thread;
}

void remove_thread_from_queue(thread_info_t** queue, thread_info_t* target) {
    if (*queue == NULL) return;
    
    if (*queue == target) {
        *queue = target->next;
        return;
    }
    
    thread_info_t* current = *queue;
    while (current->next != NULL && current->next != target) {
        current = current->next;
    }
    
    if (current->next == target) {
        current->next = target->next;
    }
}

// 创建新线程
thread_info_t* create_thread(const char* name, int priority, int burst_time, thread_vm_t vm) {
    thread_info_t* thread = malloc(sizeof(thread_info_t));
    if (!thread) return NULL;
    
    thread->thread_id = ++g_scheduler.thread_counter;
    strncpy(thread->name, name, sizeof(thread->name) - 1);
    thread->name[sizeof(thread->name) - 1] = '\0';
    thread->state = THREAD_STATE_NEW;
    thread->priority = priority;
    thread->burst_time = burst_time;
    thread->remaining_time = burst_time;
    thread->arrival_time = g_scheduler.current_time;
    thread->start_time = -1;
    thread->completion_time = -1;
    thread->waiting_time = 0;
    thread->turnaround_time = 0;
    thread->response_time = -1;
    thread->next = NULL;
    
    // 创建协程
    if (vm) {
        thread->coro = thread_create_coro(vm, 256);
    } else {
        thread->coro = NULL;
    }
    
    thread_print_info("创建线程: %s (ID:%d, 优先级:%d, 执行时间:%d)\n",
        thread->name, thread->thread_id, thread->priority, thread->burst_time);
    
    return thread;
}

// FCFS调度算法
thread_info_t* schedule_fcfs() {
    return dequeue_thread(&g_scheduler.ready_queue);
}

// SJF调度算法
thread_info_t* schedule_sjf() {
    if (g_scheduler.ready_queue == NULL) return NULL;
    
    thread_info_t* shortest = g_scheduler.ready_queue;
    thread_info_t* current = g_scheduler.ready_queue->next;
    
    while (current != NULL) {
        if (current->remaining_time < shortest->remaining_time) {
            shortest = current;
        }
        current = current->next;
    }
    
    remove_thread_from_queue(&g_scheduler.ready_queue, shortest);
    return shortest;
}

// 优先级调度算法
thread_info_t* schedule_priority() {
    if (g_scheduler.ready_queue == NULL) return NULL;
    
    thread_info_t* highest = g_scheduler.ready_queue;
    thread_info_t* current = g_scheduler.ready_queue->next;
    
    while (current != NULL) {
        if (current->priority > highest->priority) {
            highest = current;
        }
        current = current->next;
    }
    
    remove_thread_from_queue(&g_scheduler.ready_queue, highest);
    return highest;
}

// 时间片轮转调度算法
thread_info_t* schedule_round_robin() {
    return dequeue_thread(&g_scheduler.ready_queue);
}

// 执行调度算法
thread_info_t* select_next_thread() {
    switch (g_scheduler.algorithm) {
        case SCHEDULE_FCFS:
            return schedule_fcfs();
        case SCHEDULE_SJF:
            return schedule_sjf();
        case SCHEDULE_RR:
            return schedule_round_robin();
        case SCHEDULE_PRIORITY:
            return schedule_priority();
        default:
            return schedule_fcfs();
    }
}

// 执行一个时间片
void execute_time_slice() {
    if (g_scheduler.running_thread == NULL) {
        // 没有运行线程，选择新线程
        g_scheduler.running_thread = select_next_thread();
        if (g_scheduler.running_thread != NULL) {
            thread_state_t old_state = g_scheduler.running_thread->state;
            g_scheduler.running_thread->state = THREAD_STATE_RUNNING;
            
            if (g_scheduler.running_thread->start_time == -1) {
                g_scheduler.running_thread->start_time = g_scheduler.current_time;
                g_scheduler.running_thread->response_time = 
                    g_scheduler.current_time - g_scheduler.running_thread->arrival_time;
                
                // 防御性检查：响应时间不应该为负数
                if (g_scheduler.running_thread->response_time < 0) {
                    thread_print_info("警告：线程 %s 响应时间为负数 (%d)，重置为0\n", 
                        g_scheduler.running_thread->name, g_scheduler.running_thread->response_time);
                    thread_print_info("  到达时间: %d, 开始时间: %d\n",
                        g_scheduler.running_thread->arrival_time, g_scheduler.running_thread->start_time);
                    g_scheduler.running_thread->response_time = 0;
                }
            }
            
            thread_print_state_change(g_scheduler.running_thread, old_state, THREAD_STATE_RUNNING);
            thread_print_schedule_info(g_scheduler.running_thread);
            g_scheduler.context_switches++;
            g_scheduler.current_time_slice = 0;  // 重置时间片计数
        }
    }
    
    if (g_scheduler.running_thread != NULL) {
        // 执行当前线程
        g_scheduler.running_thread->remaining_time--;
        thread_print_info("执行线程 %s, 剩余时间: %d\n", 
            g_scheduler.running_thread->name, g_scheduler.running_thread->remaining_time);
        
        // 检查线程是否完成
        if (g_scheduler.running_thread->remaining_time <= 0) {
            thread_state_t old_state = g_scheduler.running_thread->state;
            g_scheduler.running_thread->state = THREAD_STATE_TERMINATED;
            // 修复：完成时间应该是当前时间+1，因为这个时间片执行完了
            g_scheduler.running_thread->completion_time = g_scheduler.current_time + 1;
            
            // 重新计算时间指标
            g_scheduler.running_thread->turnaround_time = 
                g_scheduler.running_thread->completion_time - g_scheduler.running_thread->arrival_time;
            
            // 等待时间 = 周转时间 - 实际执行时间  
            g_scheduler.running_thread->waiting_time = 
                g_scheduler.running_thread->turnaround_time - g_scheduler.running_thread->burst_time;
            
            // 确保等待时间不为负数（防御性编程）
            if (g_scheduler.running_thread->waiting_time < 0) {
                thread_print_info("警告：线程 %s 等待时间为负数，重置为0\n", g_scheduler.running_thread->name);
                g_scheduler.running_thread->waiting_time = 0;
            }
            
            thread_print_state_change(g_scheduler.running_thread, old_state, THREAD_STATE_TERMINATED);
            thread_print_info("线程 %s 完成执行详情:\n", g_scheduler.running_thread->name);
            thread_print_info("  到达时间: %d, 开始时间: %d, 完成时间: %d\n",
                g_scheduler.running_thread->arrival_time,
                g_scheduler.running_thread->start_time,
                g_scheduler.running_thread->completion_time);
            thread_print_info("  执行时间: %d, 周转时间: %d, 等待时间: %d, 响应时间: %d\n",
                g_scheduler.running_thread->burst_time,
                g_scheduler.running_thread->turnaround_time,
                g_scheduler.running_thread->waiting_time,
                g_scheduler.running_thread->response_time);
            
            enqueue_thread(&g_scheduler.terminated_queue, g_scheduler.running_thread);
            g_scheduler.running_thread = NULL;
        } else if (g_scheduler.algorithm == SCHEDULE_RR) {
            // 时间片轮转：时间片用完，线程回到就绪队列
            g_scheduler.current_time_slice++;
            
            if (g_scheduler.current_time_slice >= g_scheduler.time_quantum) {
                thread_state_t old_state = g_scheduler.running_thread->state;
                g_scheduler.running_thread->state = THREAD_STATE_READY;
                thread_print_state_change(g_scheduler.running_thread, old_state, THREAD_STATE_READY);
                
                enqueue_thread(&g_scheduler.ready_queue, g_scheduler.running_thread);
                g_scheduler.running_thread = NULL;
                g_scheduler.current_time_slice = 0;  // 重置时间片计数
            }
        }
    }
    
    g_scheduler.current_time++;
}

// 计算统计信息
void calculate_statistics() {
    int completed_threads = 0;
    int total_turnaround = 0;
    int total_waiting = 0;
    int total_response = 0;
    
    thread_info_t* current = g_scheduler.terminated_queue;
    while (current != NULL) {
        completed_threads++;
        total_turnaround += current->turnaround_time;
        total_waiting += current->waiting_time;
        total_response += current->response_time;
        current = current->next;
    }
    
    if (completed_threads > 0) {
        g_scheduler.avg_turnaround_time = (float)total_turnaround / completed_threads;
        g_scheduler.avg_waiting_time = (float)total_waiting / completed_threads;
        g_scheduler.avg_response_time = (float)total_response / completed_threads;
        
        // 计算CPU利用率（简化版）
        int total_cpu_time = 0;
        current = g_scheduler.terminated_queue;
        while (current != NULL) {
            total_cpu_time += current->burst_time;
            current = current->next;
        }
        g_scheduler.cpu_utilization = (float)total_cpu_time / g_scheduler.current_time;
    }
}

// 初始化调度器
void init_scheduler(schedule_algorithm_t algorithm, int time_quantum) {
    memset(&g_scheduler, 0, sizeof(scheduler_t));
    g_scheduler.algorithm = algorithm;
    g_scheduler.time_quantum = time_quantum;
    g_scheduler.current_time = 0;
    g_scheduler.thread_counter = 0;
    g_scheduler.context_switches = 0;
    g_scheduler.current_time_slice = 0;
    
    const char* algorithm_names[] = {"FCFS", "SJF", "RR", "Priority", "MLFQ"};
    thread_print_info("初始化调度器，算法: %s", algorithm_names[algorithm]);
    if (algorithm == SCHEDULE_RR) {
        thread_print_info(", 时间片: %d", time_quantum);
    }
    thread_print_info("\n");
}

// 添加线程到调度器
void add_thread_to_scheduler(thread_info_t* thread) {
    if (thread == NULL) return;
    
    thread_state_t old_state = thread->state;
    thread->state = THREAD_STATE_READY;
    thread_print_state_change(thread, old_state, THREAD_STATE_READY);
    
    enqueue_thread(&g_scheduler.ready_queue, thread);
}

// 运行调度模拟
void run_scheduling_simulation() {
    thread_print_info("开始线程调度模拟...\n");
    
    while (g_scheduler.ready_queue != NULL || g_scheduler.running_thread != NULL) {
        thread_print_info("--- 时间片 %d ---\n", g_scheduler.current_time);
        execute_time_slice();
        
        // 避免无限循环
        if (g_scheduler.current_time > 1000) {
            thread_print_info("模拟时间超时，强制结束\n");
            break;
        }
    }
    
    calculate_statistics();
    thread_print_statistics();
}

// 解析代码内容以确定调度参数
void parse_code_for_scheduling(const char* input_code, schedule_algorithm_t* algorithm, int* time_quantum, thread_info_t** tasks, int* task_count) {
    *algorithm = SCHEDULE_RR; // 默认时间片轮转
    *time_quantum = 2;
    *task_count = 0;
    
    if (!input_code || strlen(input_code) == 0) {
        printf("错误: 请输入线程调度代码\n");
        return;
    }
    
    // 分析代码内容，提取调度信息
    if (strstr(input_code, "FCFS") || strstr(input_code, "先来先服务")) {
        *algorithm = SCHEDULE_FCFS;
        printf("检测到FCFS调度算法\n");
    } else if (strstr(input_code, "SJF") || strstr(input_code, "最短作业")) {
        *algorithm = SCHEDULE_SJF;
        printf("检测到SJF调度算法\n");
    } else if (strstr(input_code, "优先级") || strstr(input_code, "priority")) {
        *algorithm = SCHEDULE_PRIORITY;
        printf("检测到优先级调度算法\n");
    } else if (strstr(input_code, "时间片") || strstr(input_code, "轮转") || strstr(input_code, "Round")) {
        *algorithm = SCHEDULE_RR;
        printf("检测到时间片轮转调度算法\n");
    }
    
    // 根据代码内容创建不同的任务
    if (strstr(input_code, "task_a") || strstr(input_code, "任务A")) {
        tasks[(*task_count)++] = create_thread("任务A", 1, 4, NULL);
    }
    if (strstr(input_code, "task_b") || strstr(input_code, "任务B")) {
        tasks[(*task_count)++] = create_thread("任务B", 2, 3, NULL);
    }
    if (strstr(input_code, "task_c") || strstr(input_code, "任务C")) {
        tasks[(*task_count)++] = create_thread("任务C", 1, 2, NULL);
    }
    
    // 检测高优先级任务
    if (strstr(input_code, "高优先级") || strstr(input_code, "high_priority")) {
        tasks[(*task_count)++] = create_thread("高优先级任务", 5, 2, NULL);
    }
    if (strstr(input_code, "中优先级") || strstr(input_code, "medium_priority")) {
        tasks[(*task_count)++] = create_thread("中优先级任务", 3, 4, NULL);
    }
    if (strstr(input_code, "低优先级") || strstr(input_code, "low_priority")) {
        tasks[(*task_count)++] = create_thread("低优先级任务", 1, 6, NULL);
    }
    
    // 检测不同长度的任务
    if (strstr(input_code, "短任务") || strstr(input_code, "short_task")) {
        tasks[(*task_count)++] = create_thread("短任务", 0, 2, NULL);
    }
    if (strstr(input_code, "长任务") || strstr(input_code, "long_task")) {
        tasks[(*task_count)++] = create_thread("长任务", 0, 8, NULL);
    }
    if (strstr(input_code, "中等任务") || strstr(input_code, "medium_task")) {
        tasks[(*task_count)++] = create_thread("中等任务", 0, 4, NULL);
    }
    
    // 如果没有检测到任何任务，创建默认任务
    if (*task_count == 0) {
        printf("未检测到特定任务，使用默认任务集\n");
        tasks[(*task_count)++] = create_thread("默认任务1", 1, 3, NULL);
        tasks[(*task_count)++] = create_thread("默认任务2", 2, 4, NULL);
        tasks[(*task_count)++] = create_thread("默认任务3", 1, 2, NULL);
    }
}

// Emscripten导出的演示函数
#ifdef __EMSCRIPTEN__
EMSCRIPTEN_KEEPALIVE
#endif
void thread_demo(const char* input_code) {
    printf("=== 洛书线程调度演示 ===\n");
    printf("分析输入代码...\n");
    
    if (!input_code || strlen(input_code) == 0) {
        printf("错误: 请输入线程调度代码再运行演示\n");
        printf("提示: 代码中应包含任务定义和调度算法信息\n");
        return;
    }
    
    printf("输入代码:\n%s\n", input_code);
    printf("========================\n");
    
    // 解析代码内容
    schedule_algorithm_t algorithm;
    int time_quantum;
    thread_info_t* tasks[10];
    int task_count;
    
    parse_code_for_scheduling(input_code, &algorithm, &time_quantum, tasks, &task_count);
    
    if (task_count == 0) {
        printf("错误: 未能从代码中解析出有效的任务\n");
        return;
    }
    
    // 初始化调度器
    init_scheduler(algorithm, time_quantum);
    
    // 添加解析出的线程到调度器
    // 方案1：所有线程同时到达（避免复杂的到达时间逻辑）
    for (int i = 0; i < task_count; i++) {
        // 保持所有线程的到达时间为0，确保响应时间计算正确
        tasks[i]->arrival_time = 0;
        add_thread_to_scheduler(tasks[i]);
    }
    
    // 运行调度模拟
    run_scheduling_simulation();
    
    printf("\n=== 线程调度演示完成 ===\n");
}

// FCFS演示
#ifdef __EMSCRIPTEN__
EMSCRIPTEN_KEEPALIVE
#endif
void demo_fcfs() {
    printf("=== FCFS (先来先服务) 调度演示 ===\n");
    
    init_scheduler(SCHEDULE_FCFS, 0);
    
    thread_info_t* threads[] = {
        create_thread("任务A", 0, 4, NULL),
        create_thread("任务B", 0, 3, NULL),
        create_thread("任务C", 0, 2, NULL)
    };
    
    for (int i = 0; i < 3; i++) {
        add_thread_to_scheduler(threads[i]);
    }
    
    run_scheduling_simulation();
}

// SJF演示
#ifdef __EMSCRIPTEN__
EMSCRIPTEN_KEEPALIVE
#endif
void demo_sjf() {
    printf("=== SJF (最短作业优先) 调度演示 ===\n");
    
    init_scheduler(SCHEDULE_SJF, 0);
    
    thread_info_t* threads[] = {
        create_thread("短任务", 0, 2, NULL),
        create_thread("长任务", 0, 8, NULL),
        create_thread("中任务", 0, 4, NULL)
    };
    
    for (int i = 0; i < 3; i++) {
        add_thread_to_scheduler(threads[i]);
    }
    
    run_scheduling_simulation();
}

// 优先级调度演示
#ifdef __EMSCRIPTEN__
EMSCRIPTEN_KEEPALIVE
#endif
void demo_priority() {
    printf("=== 优先级调度演示 ===\n");
    
    init_scheduler(SCHEDULE_PRIORITY, 0);
    
    thread_info_t* threads[] = {
        create_thread("低优先级", 1, 6, NULL),
        create_thread("高优先级", 5, 2, NULL),
        create_thread("中优先级", 3, 4, NULL)
    };
    
    for (int i = 0; i < 3; i++) {
        add_thread_to_scheduler(threads[i]);
    }
    
    run_scheduling_simulation();
}

// 时间片轮转演示
#ifdef __EMSCRIPTEN__
EMSCRIPTEN_KEEPALIVE
#endif
void demo_round_robin() {
    printf("=== Round Robin (时间片轮转) 调度演示 ===\n");
    
    init_scheduler(SCHEDULE_RR, 2);
    
    thread_info_t* threads[] = {
        create_thread("任务1", 0, 5, NULL),
        create_thread("任务2", 0, 3, NULL),
        create_thread("任务3", 0, 4, NULL)
    };
    
    for (int i = 0; i < 3; i++) {
        add_thread_to_scheduler(threads[i]);
    }
    
    run_scheduling_simulation();
}

// 通用运行函数
#ifdef __EMSCRIPTEN__
EMSCRIPTEN_KEEPALIVE
#endif
void run(const char* input_code) {
    thread_demo(input_code);
}

// 默认演示函数（用于测试）
#ifdef __EMSCRIPTEN__
EMSCRIPTEN_KEEPALIVE
#endif
void default_demo() {
    thread_demo("// 默认线程调度演示\n// 演示时间片轮转调度算法");
}
