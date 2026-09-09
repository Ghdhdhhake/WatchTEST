# FreeRTOS 实时调度迁移设计

## 需求

- 将本地 `FreeRTOSv9.0.0` 移植到 STM32F103C8T6、Keil ARMCC5 工程。
- 编码器输入必须持续采样，不能因 OLED 刷屏、动画或传感器读取而漏按。
- 秒表和恐龙游戏保持 1 ms 软件时基。
- 保留现有 UI 和硬件驱动，控制 20 KB SRAM 占用。

## 方案比较

1. 单任务包装现有主循环：改动最少，但阻塞式页面仍会影响输入，不满足实时响应。
2. UI、输入、时基三个任务：输入通过队列传递事件，OLED 只由 UI 任务访问，兼顾实时性和迁移风险。本项目采用此方案。
3. 每个页面一个任务：隔离更彻底，但任务生命周期、OLED 互斥和状态切换复杂，现阶段属于过度设计。

## 架构

```text
Encoder GPIO -> InputTask (2 ms, P3) -> event queue -> UiTask (P2) -> OLED
FreeRTOS tick -> TimebaseTask (1 ms, P4) -> StopWatch_Tick / Dino_Tick
IdleTask (P0)
```

UI 任务是 OLED 和菜单状态的唯一所有者，不需要显示互斥锁。输入任务是编码器状态机的唯一调用者，将左转、右转、按下事件写入长度为 8 的队列。时基任务使用 `vTaskDelayUntil()`，避免原 TIM2 中断与 RTOS SysTick 重复维护两个 1 ms 时基。

## 关键决定（ADR）

- 使用 FreeRTOS V9 的 RVDS/ARM_CM3 端口，因为工程编译器是 ARMCC 5.06。
- 使用 `heap_4.c` 和 10 KB RTOS 堆；三个应用任务加空闲任务预计使用约 5–7 KB。
- SysTick、PendSV、SVC 由 FreeRTOS 端口接管，删除模板中的空异常处理函数。
- `Delay_ms()` 在调度器运行后调用 `vTaskDelay()`；微秒延时改用 Cortex-M3 DWT 周期计数器，避免破坏 RTOS SysTick。
- FreeRTOS API 不在现有外设 ISR 中调用；NVIC 使用全抢占优先级分组。

## 失败模式与保护

- 任务或队列创建失败：启动前停机，便于调试器定位。
- 栈溢出、堆分配失败：进入断言停机钩子。
- 输入队列满：丢弃最旧事件后写入最新事件，保证操作不会长期滞后。
- OLED 并发：仅 UI 任务访问，避免软件 I2C 时序和帧缓冲竞争。

