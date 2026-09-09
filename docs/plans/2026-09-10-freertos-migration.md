# FreeRTOS Migration Implementation Plan

> **For Claude:** REQUIRED SUB-SKILL: Use superpowers:executing-plans to implement this plan task-by-task.

**Goal:** 将 FreeRTOS V9.0.0 集成到 STM32F103 手表工程，并用独立输入、时基和 UI 任务提高实时响应。

**Architecture:** 使用 RVDS Cortex-M3 端口和 heap_4。高优先级输入任务采样编码器并写事件队列，高优先级时基任务维护 1 ms 软件节拍，UI 任务独占 OLED 和页面状态。

**Tech Stack:** STM32F103C8T6、STM32F10x StdPeriph、FreeRTOS V9.0.0、Keil ARMCC5

---

### Task 1: 集成 FreeRTOS 内核

**Files:**
- Create: `FreeRTOS/Source/*`
- Create: `User/FreeRTOSConfig.h`
- Modify: `Project.uvprojx`

1. 从本地资料复制 `tasks.c`、`queue.c`、`list.c`、公共头文件、RVDS/ARM_CM3 端口和 `heap_4.c`。
2. 配置 72 MHz、1 kHz tick、10 KB heap、5 个优先级以及 Cortex-M3 中断映射。
3. 将文件和 include 路径加入 Keil 工程。
4. 解析 `Project.uvprojx`，确保 XML 有效且每个源文件只出现一次。

### Task 2: 建立实时任务与输入队列

**Files:**
- Create: `User/AppTasks.c`
- Create: `User/AppTasks.h`
- Modify: `Hardware/encoder.c`
- Modify: `Hardware/encoder.h`
- Modify: `Hardware/dino.c`

1. 添加 2 ms 编码器采样任务和长度 8 的事件队列。
2. 让现有 `Encoder_GetKeyNum()` 从队列非阻塞取事件，保持页面 API 不变。
3. 添加 1 ms `vTaskDelayUntil()` 时基任务。
4. 添加 UI 任务并迁移原主循环。
5. 对任务创建结果进行检查。

### Task 3: 处理内核时基冲突

**Files:**
- Modify: `System/Delay.c`
- Modify: `System/Delay.h`
- Modify: `User/stm32f10x_it.c`
- Modify: `User/main.c`
- Delete: `System/Timer.c`
- Delete: `System/Timer.h`

1. 用 DWT 实现微秒延时。
2. 调度器运行后让毫秒延时阻塞当前任务，而不是占用 SysTick。
3. 移除 TIM2 软件时基和旧中断函数。
4. 移除空的 SVC、PendSV、SysTick 模板处理函数。
5. 初始化外设、创建任务并启动调度器。

### Task 4: 验证与文档

**Files:**
- Modify: `ARCHITECTURE.md`

1. 使用 ARM GCC 对通用 C 文件进行语法检查。
2. 使用 ARMCC/Keil 构建工程（若本机可执行文件可用）。
3. 检查异常处理符号、旧 TIM2 引用和 FreeRTOS 配置一致性。
4. 记录上板验证步骤：编码器快速操作、动态表情退出、秒表精度、恐龙动画和长时间运行。

