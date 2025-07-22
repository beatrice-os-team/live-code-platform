# 基于洛书（Losu）编程语言与 Web Assembly 的编译与 OS 演示系统

---

## 项目简介

本项目基于洛书（Losu）编程语言与 Web Assembly 技术，打造一个集编程教学与操作系统原理演示于一体的交互式网站。用户可以在浏览器中编写 Losu 代码，体验编译、虚拟机运行、内存管理、线程调度、文件操作等底层 OS 功能的可视化过程，助力系统能力学习与创新实践。

本项目为 2025 年全国大学生计算机系统能力大赛-操作系统设计赛（全国）- OS功能挑战赛道参赛作品。

## 中断处理实验介绍

### 实验概述

中断处理是操作系统的核心机制之一，它允许系统响应外部事件和管理程序执行流程。本实验通过洛书虚拟机的协程机制和 WebAssembly 的事件处理，模拟中断处理的基本原理，帮助学生理解中断驱动的程序设计模式。

### 学习目标

- **理解中断的概念**：掌握中断在操作系统中的重要作用
- **掌握中断处理流程**：学习中断发生、处理和恢复的完整过程
- **了解协程与中断的关系**：理解协程如何模拟中断处理机制
- **体验事件驱动编程**：通过实际代码体验事件驱动的程序设计

### 实验内容

#### 1. 中断处理基本概念

**中断类型**
- **硬件中断**：由硬件设备触发的异步事件
  - 时钟中断：定时器产生的周期性中断
  - I/O中断：输入输出设备完成操作时的通知
  - 键盘中断：用户按键时产生的中断
- **软件中断**：由程序主动触发的同步事件
  - 系统调用：用户程序请求操作系统服务
  - 异常处理：程序错误或特殊条件的处理
  - 信号处理：进程间通信的机制

**中断处理过程**
1. **中断发生**：外部事件或内部条件触发中断
2. **保存上下文**：保存当前程序的执行状态
3. **中断识别**：确定中断源和中断类型
4. **中断处理**：执行相应的中断服务程序
5. **恢复上下文**：恢复被中断程序的执行状态
6. **返回执行**：继续执行被中断的程序

#### 2. 洛书虚拟机的中断模拟

**协程中断机制**
洛书虚拟机通过协程的 `yield` 机制模拟中断处理：

```losu
# 协程模拟中断处理
async def interrupt_handler(interrupt_type, data):
    print(f"处理中断: {interrupt_type}")
    # 执行中断处理逻辑
    yield  # 让出控制权，模拟中断处理时间
    print(f"中断处理完成: {interrupt_type}")

# 主程序
async def main_program():
    for i in 1, 10:
        print(f"主程序执行: 步骤 {i}")
        
        # 模拟中断发生
        if i % 3 == 0:
            async interrupt_handler("TIMER", i)
        
        yield  # 协作式多任务
```

**事件驱动模型**
```losu
class EventSystem:
    def __init__(self):
        self.handlers = {}
        self.event_queue = []
    
    def register_handler(self, event_type, handler):
        if event_type not in self.handlers:
            self.handlers[event_type] = []
        self.handlers[event_type].append(handler)
    
    def trigger_event(self, event_type, data):
        self.event_queue.append({"type": event_type, "data": data})
    
    async def process_events(self):
        while self.event_queue.length() > 0:
            let event = self.event_queue.pop(0)
            await self.handle_event(event.type, event.data)
            yield
    
    async def handle_event(self, event_type, data):
        if event_type in self.handlers:
            for handler in self.handlers[event_type]:
                await handler(data)
                yield
```

#### 3. 中断处理实验场景

**时钟中断模拟**
```losu
import time

class TimerInterrupt:
    def __init__(self, interval):
        self.interval = interval
        self.last_time = time::now()
        self.tick_count = 0
    
    def check_interrupt(self):
        let current_time = time::now()
        if current_time - self.last_time >= self.interval:
            self.last_time = current_time
            return true
        return false
    
    async def timer_handler(self):
        self.tick_count += 1
        print(f"时钟中断 #{self.tick_count} - 时间: {time::now()}")
        yield

# 使用时钟中断
let timer = TimerInterrupt(1000)  # 1秒间隔

async def main_with_timer():
    for i in 1, 20:
        print(f"主程序工作: {i}")
        
        # 检查时钟中断
        if timer.check_interrupt():
            async timer.timer_handler()
        
        yield
```

**I/O中断模拟**
```losu
class IODevice:
    def __init__(self, name):
        self.name = name
        self.busy = false
        self.completion_time = null
    
    def start_operation(self, duration):
        self.busy = true
        self.completion_time = time::now() + duration
        print(f"{self.name} 开始 I/O 操作，预计耗时 {duration}ms")
    
    def check_completion(self):
        if self.busy and time::now() >= self.completion_time:
            self.busy = false
            return true
        return false
    
    async def completion_handler(self, data):
        print(f"{self.name} I/O 操作完成，数据: {data}")
        yield

# I/O中断处理示例
async def io_interrupt_demo():
    let disk = IODevice("磁盘")
    let network = IODevice("网络")
    
    # 启动I/O操作
    disk.start_operation(2000)
    network.start_operation(1500)
    
    let step = 0
    while disk.busy or network.busy:
        step += 1
        print(f"CPU执行其他任务: 步骤 {step}")
        
        # 检查I/O完成中断
        if disk.check_completion():
            async disk.completion_handler("文件数据")
        
        if network.check_completion():
            async network.completion_handler("网络数据包")
        
        yield
```

**信号处理模拟**
```losu
class SignalSystem:
    def __init__(self):
        self.signal_handlers = {}
        self.pending_signals = []
    
    def register_signal(self, signal_type, handler):
        self.signal_handlers[signal_type] = handler
    
    def send_signal(self, signal_type, data=null):
        self.pending_signals.append({
            "type": signal_type,
            "data": data,
            "timestamp": time::now()
        })
    
    async def process_signals(self):
        while self.pending_signals.length() > 0:
            let signal = self.pending_signals.pop(0)
            await self.handle_signal(signal)
            yield
    
    async def handle_signal(self, signal):
        print(f"处理信号: {signal.type}")
        if signal.type in self.signal_handlers:
            await self.signal_handlers[signal.type](signal.data)
        yield

# 信号处理示例
async def signal_handler_demo():
    let signal_system = SignalSystem()
    
    # 注册信号处理器
    signal_system.register_signal("SIGTERM", async def(data):
        print("收到终止信号，开始清理...")
        yield
    )
    
    signal_system.register_signal("SIGUSR1", async def(data):
        print(f"收到用户信号1: {data}")
        yield
    )
    
    # 模拟程序运行
    for i in 1, 10:
        print(f"程序运行: {i}")
        
        # 模拟信号发送
        if i == 3:
            signal_system.send_signal("SIGUSR1", "测试数据")
        if i == 7:
            signal_system.send_signal("SIGTERM")
        
        # 处理待处理的信号
        async signal_system.process_signals()
        yield
```

#### 4. 中断优先级和嵌套

**优先级中断处理**
```losu
class PriorityInterruptSystem:
    def __init__(self):
        self.interrupt_queue = []
        self.current_priority = 0
    
    def add_interrupt(self, type, priority, handler):
        self.interrupt_queue.append({
            "type": type,
            "priority": priority,
            "handler": handler,
            "timestamp": time::now()
        })
        # 按优先级排序（优先级数字越小越高）
        self.interrupt_queue.sort(key=lambda x: x.priority)
    
    async def process_interrupts(self):
        while self.interrupt_queue.length() > 0:
            let interrupt = self.interrupt_queue.pop(0)
            
            # 检查优先级是否足够高
            if interrupt.priority <= self.current_priority:
                print(f"处理高优先级中断: {interrupt.type} (优先级 {interrupt.priority})")
                let old_priority = self.current_priority
                self.current_priority = interrupt.priority
                
                await interrupt.handler()
                
                self.current_priority = old_priority
                print(f"恢复到优先级: {old_priority}")
            else:
                # 优先级不够，重新加入队列
                self.interrupt_queue.append(interrupt)
                break
            yield

# 优先级中断示例
async def priority_interrupt_demo():
    let system = PriorityInterruptSystem()
    
    # 添加不同优先级的中断
    system.add_interrupt("键盘", 3, async def():
        print("处理键盘输入")
        yield
    )
    
    system.add_interrupt("网络", 2, async def():
        print("处理网络数据包")
        yield
    )
    
    system.add_interrupt("时钟", 1, async def():
        print("处理时钟中断")
        yield
    )
    
    # 处理中断队列
    async system.process_interrupts()
```

#### 5. 异常处理机制

**异常类型模拟**
```losu
class ExceptionHandler:
    def __init__(self):
        self.exception_handlers = {}
    
    def register_exception(self, exception_type, handler):
        self.exception_handlers[exception_type] = handler
    
    async def handle_exception(self, exception_type, context):
        print(f"发生异常: {exception_type}")
        
        if exception_type in self.exception_handlers:
            await self.exception_handlers[exception_type](context)
        else:
            print(f"未处理的异常: {exception_type}")
        yield

# 异常处理示例
async def exception_demo():
    let handler = ExceptionHandler()
    
    # 注册异常处理器
    handler.register_exception("DivideByZero", async def(context):
        print("除零异常处理: 返回默认值")
        context["result"] = 0
        yield
    )
    
    handler.register_exception("NullPointer", async def(context):
        print("空指针异常处理: 创建默认对象")
        context["object"] = {}
        yield
    )
    
    # 模拟异常发生
    let context = {}
    async handler.handle_exception("DivideByZero", context)
    print(f"异常处理结果: {context.result}")
```

### 操作步骤

1. **访问中断处理页面**：在主页侧边栏点击"中断处理"进入实验页面
2. **了解中断类型**：学习不同类型中断的特点和处理方式
3. **选择实验场景**：从预设的中断处理示例中选择，或编写自定义代码
4. **执行中断模拟**：运行洛书代码，观察中断处理过程
5. **监控中断状态**：查看中断处理的详细信息：
   - 中断发生时间
   - 处理优先级
   - 执行时间
   - 上下文切换开销
6. **分析性能影响**：评估中断处理对程序性能的影响

### 实验重点

#### 中断机制理解
- **异步性质**：理解中断的异步特性和处理方式
- **上下文切换**：掌握中断时的状态保存和恢复
- **优先级管理**：了解中断优先级的重要性

#### 协程模拟
- **协作式多任务**：通过 `yield` 实现控制权转移
- **状态保持**：协程如何保持执行状态
- **调度机制**：协程调度器的工作原理

#### 事件驱动
- **事件循环**：理解事件驱动编程模型
- **回调机制**：掌握回调函数的使用
- **异步处理**：学习异步编程的设计模式

### 技术实现

- **协程调度器**：基于洛书虚拟机的协程调度机制
- **事件队列**：实现事件的排队和处理
- **优先级队列**：支持优先级中断处理
- **状态管理**：维护中断处理的上下文状态

### 中断处理优化

**减少中断开销**
- 中断合并：合并同类型的多个中断
- 延迟处理：将复杂处理推迟到中断处理程序之外
- 硬件加速：利用硬件特性减少软件开销

**提高响应性**
- 短中断处理程序：保持中断处理程序简短
- 优先级调度：根据重要性安排中断处理顺序
- 预测性处理：预测可能的中断并提前准备

### 实验价值

通过中断处理实验，学生将：
- 深入理解操作系统的中断处理机制
- 掌握事件驱动编程的基本技巧
- 了解异步编程和协程的工作原理
- 学会设计响应式的系统架构

### 扩展思考

1. **中断与轮询的权衡**：何时使用中断，何时使用轮询
2. **实时系统设计**：如何在实时系统中保证中断响应时间
3. **多核系统中断**：多核环境下的中断分发和处理
4. **虚拟化中断**：虚拟机环境下的中断虚拟化技术

通过本实验，学生将全面了解中断处理的设计原理和实现技术，为系统编程和实时应用开发奠定基础。 