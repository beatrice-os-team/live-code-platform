# 基于洛书（Losu）编程语言与 Web Assembly 的编译与 OS 演示系统

---

## 项目简介

本项目基于洛书（Losu）编程语言与 Web Assembly 技术，打造一个集编程教学与操作系统原理演示于一体的交互式网站。用户可以在浏览器中编写 Losu 代码，体验编译、虚拟机运行、内存管理、线程调度、文件操作等底层 OS 功能的可视化过程，助力系统能力学习与创新实践。

本项目为 2025 年全国大学生计算机系统能力大赛-操作系统设计赛（全国）- OS功能挑战赛道参赛作品。

## 线程调度实验介绍

### 实验概述

线程调度是操作系统多任务处理的核心机制，决定了系统的并发性能和响应能力。本实验通过洛书虚拟机的协程系统模拟线程调度算法，帮助学生理解不同调度策略的原理、特点和适用场景，体验多任务环境下的程序设计。

### 学习目标

- **理解线程调度的重要性**：掌握线程调度在操作系统中的关键作用
- **掌握调度算法原理**：学习各种线程调度算法的工作机制
- **了解协程与线程的关系**：理解协程如何模拟真实的线程调度
- **体验并发编程**：通过实际代码体验多任务编程的挑战和技巧

### 实验内容

#### 1. 线程调度基本概念

**调度的目标**
- **公平性**：确保所有线程都能获得合理的执行机会
- **效率**：最大化系统的整体吞吐量
- **响应性**：保证交互式任务的快速响应
- **实时性**：满足实时任务的时间约束

**线程状态转换**
```
就绪态 (Ready) ←→ 运行态 (Running)
    ↑                    ↓
    ←── 阻塞态 (Blocked) ←──
```

**调度时机**
- 时间片到期：当前线程的时间片用完
- 主动让出：线程主动调用 yield 或阻塞操作
- 高优先级线程就绪：更高优先级的线程变为就绪状态
- 系统调用返回：从内核态返回用户态时

#### 2. 洛书协程调度系统

**协程调度器**
洛书虚拟机通过协程调度器模拟线程调度：

```losu
class Scheduler:
    def __init__(self):
        self.ready_queue = []
        self.running = null
        self.blocked = []
        self.total_time = 0
    
    def add_task(self, task):
        task.state = "ready"
        task.create_time = self.total_time
        self.ready_queue.append(task)
    
    def schedule(self):
        # 抽象调度方法，由具体调度算法实现
        pass
    
    async def run(self):
        while self.ready_queue.length() > 0 or self.blocked.length() > 0:
            self.schedule()  # 选择下一个要执行的任务
            if self.running:
                await self.execute_task()
            self.total_time += 1
            yield
```

**任务表示**
```losu
class Task:
    def __init__(self, name, priority=0, burst_time=1):
        self.name = name
        self.priority = priority
        self.burst_time = burst_time
        self.remaining_time = burst_time
        self.state = "new"
        self.create_time = 0
        self.start_time = 0
        self.completion_time = 0
        self.wait_time = 0
        self.response_time = -1
    
    async def execute(self):
        # 任务的实际执行逻辑
        yield
```

#### 3. 调度算法实现

**先来先服务 (FCFS)**
```losu
class FCFSScheduler(Scheduler):
    def schedule(self):
        if not self.running and self.ready_queue.length() > 0:
            # 选择最早到达的任务
            self.running = self.ready_queue.pop(0)
            self.running.state = "running"
            if self.running.response_time == -1:
                self.running.response_time = self.total_time - self.running.create_time

# FCFS调度示例
async def fcfs_demo():
    let scheduler = FCFSScheduler()
    
    # 添加任务
    scheduler.add_task(Task("任务A", 0, 4))
    scheduler.add_task(Task("任务B", 0, 3))
    scheduler.add_task(Task("任务C", 0, 2))
    
    print("开始FCFS调度")
    async scheduler.run()
```

**最短作业优先 (SJF)**
```losu
class SJFScheduler(Scheduler):
    def schedule(self):
        if not self.running and self.ready_queue.length() > 0:
            # 选择剩余时间最短的任务
            self.ready_queue.sort(key=lambda task: task.remaining_time)
            self.running = self.ready_queue.pop(0)
            self.running.state = "running"
            if self.running.response_time == -1:
                self.running.response_time = self.total_time - self.running.create_time

# SJF调度示例
async def sjf_demo():
    let scheduler = SJFScheduler()
    
    scheduler.add_task(Task("短任务", 0, 2))
    scheduler.add_task(Task("长任务", 0, 8))
    scheduler.add_task(Task("中任务", 0, 4))
    
    print("开始SJF调度")
    async scheduler.run()
```

**时间片轮转 (Round Robin)**
```losu
class RoundRobinScheduler(Scheduler):
    def __init__(self, time_quantum):
        super().__init__()
        self.time_quantum = time_quantum
        self.current_quantum = 0
    
    def schedule(self):
        # 时间片到期或当前没有运行任务
        if (self.running and self.current_quantum >= self.time_quantum) or not self.running:
            if self.running:
                # 当前任务回到就绪队列末尾
                self.running.state = "ready"
                self.ready_queue.append(self.running)
            
            if self.ready_queue.length() > 0:
                self.running = self.ready_queue.pop(0)
                self.running.state = "running"
                self.current_quantum = 0
                if self.running.response_time == -1:
                    self.running.response_time = self.total_time - self.running.create_time
            else:
                self.running = null
    
    async def execute_task(self):
        if self.running:
            print(f"执行任务: {self.running.name} (剩余时间: {self.running.remaining_time})")
            self.running.remaining_time -= 1
            self.current_quantum += 1
            
            if self.running.remaining_time <= 0:
                self.running.state = "completed"
                self.running.completion_time = self.total_time + 1
                print(f"任务 {self.running.name} 完成")
                self.running = null
                self.current_quantum = 0
        yield

# Round Robin调度示例
async def rr_demo():
    let scheduler = RoundRobinScheduler(2)  # 时间片为2
    
    scheduler.add_task(Task("任务1", 0, 5))
    scheduler.add_task(Task("任务2", 0, 3))
    scheduler.add_task(Task("任务3", 0, 4))
    
    print("开始Round Robin调度")
    async scheduler.run()
```

**优先级调度**
```losu
class PriorityScheduler(Scheduler):
    def __init__(self, preemptive=false):
        super().__init__()
        self.preemptive = preemptive
    
    def schedule(self):
        # 抢占式：如果有更高优先级的任务就绪，立即切换
        if self.preemptive and self.running:
            let highest_priority = max(self.ready_queue, key=lambda task: task.priority)
            if highest_priority.priority > self.running.priority:
                self.running.state = "ready"
                self.ready_queue.append(self.running)
                self.running = highest_priority
                self.ready_queue.remove(highest_priority)
                return
        
        # 非抢占式：只有当前任务完成才调度新任务
        if not self.running and self.ready_queue.length() > 0:
            # 选择优先级最高的任务
            self.ready_queue.sort(key=lambda task: task.priority, reverse=true)
            self.running = self.ready_queue.pop(0)
            self.running.state = "running"
            if self.running.response_time == -1:
                self.running.response_time = self.total_time - self.running.create_time

# 优先级调度示例
async def priority_demo():
    let scheduler = PriorityScheduler(true)  # 抢占式
    
    scheduler.add_task(Task("低优先级", 1, 6))
    scheduler.add_task(Task("高优先级", 5, 2))
    scheduler.add_task(Task("中优先级", 3, 4))
    
    print("开始优先级调度")
    async scheduler.run()
```

**多级反馈队列 (MLFQ)**
```losu
class MLFQScheduler(Scheduler):
    def __init__(self, num_levels, time_quanta):
        super().__init__()
        self.num_levels = num_levels
        self.time_quanta = time_quanta
        self.queues = [[] for i 1, num_levels]
        self.current_level = 0
        self.current_quantum = 0
    
    def add_task(self, task):
        task.state = "ready"
        task.create_time = self.total_time
        task.level = 0  # 新任务从最高优先级队列开始
        self.queues[0].append(task)
    
    def schedule(self):
        # 如果当前任务时间片用完，降级到下一级队列
        if self.running and self.current_quantum >= self.time_quanta[self.current_level]:
            self.running.state = "ready"
            if self.running.level < self.num_levels - 1:
                self.running.level += 1
            self.queues[self.running.level].append(self.running)
            self.running = null
            self.current_quantum = 0
        
        # 选择新任务执行
        if not self.running:
            for level in 1, self.num_levels:
                if self.queues[level].length() > 0:
                    self.running = self.queues[level].pop(0)
                    self.running.state = "running"
                    self.current_level = level
                    self.current_quantum = 0
                    if self.running.response_time == -1:
                        self.running.response_time = self.total_time - self.running.create_time
                    break

# MLFQ调度示例
async def mlfq_demo():
    let scheduler = MLFQScheduler(3, [1, 2, 4])  # 3级队列，时间片递增
    
    scheduler.add_task(Task("交互任务", 0, 3))
    scheduler.add_task(Task("CPU密集任务", 0, 8))
    scheduler.add_task(Task("混合任务", 0, 5))
    
    print("开始多级反馈队列调度")
    async scheduler.run()
```

#### 4. 调度算法比较实验

**性能指标计算**
```losu
class SchedulingAnalyzer:
    def __init__(self):
        self.tasks = []
    
    def add_task(self, task):
        self.tasks.append(task)
    
    def calculate_metrics(self):
        let total_turnaround = 0
        let total_waiting = 0
        let total_response = 0
        let completed_tasks = 0
        
        for task in self.tasks:
            if task.state == "completed":
                let turnaround_time = task.completion_time - task.create_time
                let waiting_time = turnaround_time - task.burst_time
                total_turnaround += turnaround_time
                total_waiting += waiting_time
                total_response += task.response_time
                completed_tasks += 1
        
        if completed_tasks > 0:
            return {
                "平均周转时间": total_turnaround / completed_tasks,
                "平均等待时间": total_waiting / completed_tasks,
                "平均响应时间": total_response / completed_tasks,
                "完成任务数": completed_tasks
            }
        return {}

# 调度算法对比实验
async def scheduling_comparison():
    let task_sets = [
        [Task("A", 0, 4), Task("B", 0, 3), Task("C", 0, 2)],
        [Task("短", 0, 1), Task("长", 0, 10), Task("中", 0, 5)],
        [Task("高", 5, 3), Task("低", 1, 6), Task("中", 3, 4)]
    ]
    
    let algorithms = [
        ("FCFS", FCFSScheduler()),
        ("SJF", SJFScheduler()),
        ("RR", RoundRobinScheduler(2)),
        ("优先级", PriorityScheduler(true))
    ]
    
    for task_set in task_sets:
        print(f"任务集: {[task.name for task in task_set]}")
        
        for name, scheduler in algorithms:
            # 重置调度器和任务
            scheduler.__init__()
            for task in task_set:
                task_copy = Task(task.name, task.priority, task.burst_time)
                scheduler.add_task(task_copy)
            
            # 运行调度
            await scheduler.run()
            
            # 计算性能指标
            let analyzer = SchedulingAnalyzer()
            for task in scheduler.tasks:
                analyzer.add_task(task)
            
            let metrics = analyzer.calculate_metrics()
            print(f"{name} 算法: {metrics}")
        
        print("---")
```

#### 5. 实时调度和死锁

**实时调度示例**
```losu
class RTTask(Task):
    def __init__(self, name, period, deadline, computation_time):
        super().__init__(name, 0, computation_time)
        self.period = period
        self.deadline = deadline
        self.next_release = 0
        self.missed_deadlines = 0
    
    def is_ready(self, current_time):
        return current_time >= self.next_release
    
    def check_deadline(self, current_time):
        if current_time > self.next_release + self.deadline:
            self.missed_deadlines += 1
            return false
        return true

class RateMonotonicScheduler(Scheduler):
    def schedule(self):
        # 按周期排序（Rate Monotonic算法）
        let rt_tasks = [task for task in self.ready_queue if isinstance(task, RTTask)]
        rt_tasks.sort(key=lambda task: task.period)
        
        if rt_tasks.length() > 0:
            self.running = rt_tasks[0]
            self.ready_queue.remove(self.running)
            self.running.state = "running"

# 实时调度示例
async def real_time_demo():
    let scheduler = RateMonotonicScheduler()
    
    scheduler.add_task(RTTask("任务1", 4, 3, 1))  # 周期4，截止时间3，计算时间1
    scheduler.add_task(RTTask("任务2", 6, 5, 2))  # 周期6，截止时间5，计算时间2
    
    print("开始实时调度")
    async scheduler.run()
```

**死锁检测**
```losu
class Resource:
    def __init__(self, name):
        self.name = name
        self.owner = null
        self.waiting_queue = []
    
    def request(self, task):
        if self.owner == null:
            self.owner = task
            return true
        else:
            self.waiting_queue.append(task)
            return false
    
    def release(self, task):
        if self.owner == task:
            self.owner = null
            if self.waiting_queue.length() > 0:
                let next_task = self.waiting_queue.pop(0)
                self.owner = next_task
                return next_task
        return null

class DeadlockDetector:
    def __init__(self):
        self.resources = {}
        self.allocation = {}
        self.request = {}
    
    def detect_deadlock(self):
        # 银行家算法或等待图检测
        # 简化实现：检查循环等待
        pass

# 死锁演示
async def deadlock_demo():
    let resource1 = Resource("资源1")
    let resource2 = Resource("资源2")
    
    async def task_a():
        print("任务A请求资源1")
        resource1.request("任务A")
        yield
        print("任务A请求资源2")
        if not resource2.request("任务A"):
            print("任务A等待资源2（可能死锁）")
        yield
    
    async def task_b():
        print("任务B请求资源2")
        resource2.request("任务B")
        yield
        print("任务B请求资源1")
        if not resource1.request("任务B"):
            print("任务B等待资源1（可能死锁）")
        yield
    
    # 并发执行可能导致死锁
    async task_a()
    async task_b()
```

### 操作步骤

1. **访问线程调度页面**：在主页侧边栏点击"线程调度"进入实验页面
2. **了解调度算法**：学习不同调度算法的原理和特点
3. **选择实验场景**：从预设的调度算法示例中选择，或编写自定义任务集
4. **执行调度模拟**：运行洛书代码，观察任务调度过程
5. **监控调度状态**：查看调度的详细信息：
   - 任务状态变化
   - 调度时机和决策
   - 性能指标统计
   - 资源使用情况
6. **比较调度效果**：对比不同算法在相同任务集下的表现

### 实验重点

#### 调度算法理解
- **算法特性**：理解各种调度算法的优缺点
- **适用场景**：掌握不同算法的适用环境
- **性能权衡**：了解公平性、效率和响应性的权衡

#### 协程调度
- **协作式调度**：理解协程的协作式调度机制
- **状态管理**：掌握任务状态的转换和管理
- **调度策略**：学习如何实现不同的调度策略

#### 性能分析
- **评价指标**：理解周转时间、等待时间等指标
- **瓶颈识别**：学会识别调度系统的性能瓶颈
- **优化方法**：掌握调度性能的优化技巧

### 技术实现

- **协程调度器**：基于洛书虚拟机的协程调度实现
- **任务队列管理**：实现多级队列和优先级队列
- **时间模拟**：模拟时间片和任务执行时间
- **性能监控**：收集和分析调度性能数据

### 调度优化策略

**负载均衡**
- 多处理器调度：在多核环境下分配任务
- 工作窃取：空闲处理器从忙碌处理器窃取任务
- 亲和性调度：考虑缓存局部性的任务分配

**自适应调度**
- 动态优先级调整：根据任务行为调整优先级
- 学习型调度：根据历史数据优化调度决策
- 反馈控制：根据系统负载调整调度参数

### 实验价值

通过线程调度实验，学生将：
- 深入理解操作系统的调度机制
- 掌握多任务编程的设计原则
- 了解并发系统的性能优化方法
- 学会分析和解决调度相关的问题

### 扩展思考

1. **调度公平性**：如何在效率和公平性之间找到平衡
2. **多核调度**：多核环境下的调度算法设计
3. **虚拟化调度**：虚拟机环境下的调度优化
4. **能耗感知调度**：考虑功耗的调度算法设计

通过本实验，学生将全面了解线程调度的设计原理和实现技术，为并发编程和系统优化奠定基础。 