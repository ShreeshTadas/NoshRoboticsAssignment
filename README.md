# NoshRoboticsAssignment

Embedded Systems Assignment for Internship at Nosh Robotics

**Candidate:** Shreesh Purandarvittal Tadas

This repository contains comprehensive solutions for the embedded systems engineering assignment for Nosh Robotics. The project is divided into two distinct parts:

1. **Q1**: A bare-metal interrupt-driven state machine on the STM32G070RB microcontroller
2. **Q2**: A thread-safe producer-consumer simulation demonstrating concurrent embedded systems design

---

## Table of Contents

- [Project Overview](#project-overview)
- [Question 1: STM32G070RB Switch & LED State Machine](#question-1-stm32g070rb-switch--led-state-machine)
  - [Overview](#q1-overview)
  - [Hardware Mapping](#hardware-mapping)
  - [Firmware Architecture](#firmware-architecture--power-optimization)
  - [How to Build](#how-to-build-q1)
  - [Expected Behavior](#q1-expected-behavior)
- [Question 2: Online C Compiler Sensor Simulation](#question-2-online-c-compiler-sensor-simulation)
  - [Overview](#q2-overview)
  - [How to Run](#how-to-run-q2)
  - [System Architecture](#system-architecture)
  - [Design Assumptions](#design-assumptions)
  - [Expected Behavior](#q2-expected-behavior)
- [Requirements](#requirements)
- [Project Structure](#project-structure)

---

## Project Overview

This assignment demonstrates proficiency in two critical domains of embedded systems engineering:

- **Q1 - Firmware Development**: Low-power, interrupt-driven real-time systems on ARM Cortex-M0+ processors with dynamic peripheral reconfiguration
- **Q2 - Concurrency & Thread Safety**: Producer-consumer patterns with mutex synchronization and thread-safe data structures

---

## Question 1: STM32G070RB Switch & LED State Machine

### Q1 Overview

This project implements a low-power, interrupt-driven state machine on the NUCLEO-G070RB board. A single tactile switch cycles through four distinct LED blinking frequencies: **0.5 Hz, 1.0 Hz, 2.0 Hz, and 4.0 Hz**, creating an elegant demonstration of real-time embedded systems design principles.

**Key Design Goals:**
- Zero busy-waiting (no polling loops)
- Minimal power consumption via sleep modes
- Non-blocking debouncing
- Efficient peripheral usage

### Hardware Mapping

> **Note:** This firmware is designed to utilize the existing on-board peripherals of the NUCLEO-G070RB, requiring no external breadboard circuitry.

| Component | Pin | Configuration |
|-----------|-----|----------------|
| **User Button** | PC13 | External Interrupt (EXTI), falling edge trigger (active-low) |
| **User LED** | PA5 | GPIO Push-Pull output (green on-board LED) |
| **Timebase** | TIM6 | Timer configured with prescaler to generate 1ms tick |

### Firmware Architecture & Power Optimization

To strictly adhere to the power optimization requirement, this firmware avoids blocking loops and polling entirely.

#### Sleep Mode Implementation
Instead of spinning in a `while(1)` loop, the MCU executes:
```c
HAL_PWR_EnterSLEEPMode(PWR_MAINREGULATOR_ON, PWR_SLEEPENTRY_WFI);
```
This halts the ARM Cortex core clock while keeping peripherals operational, dramatically reducing power consumption during idle periods.

#### SysTick Management
Before entering Sleep Mode, `HAL_SuspendTick()` is called so the 1ms SysTick interrupt doesn't unnecessarily wake the core. `HAL_ResumeTick()` is immediately called upon waking (inside the button interrupt handler), ensuring accurate timing only when needed.

#### Non-Blocking Debounce
Inside the EXTI callback, a 50ms software debounce is implemented using `HAL_GetTick()`. Unlike `HAL_Delay()`, this is entirely non-blocking and prevents ISR deadlocks if interrupt priorities are misconfigured.

#### Dynamic Timer Reconfiguration
Rather than running multiple timers, the Auto-Reload Register (ARR) of TIM6 is dynamically updated on each button press. The timer counter is explicitly reset using `__HAL_TIM_SET_COUNTER()`, ensuring immediate frequency transitions without phase discontinuities.

**Reconfiguration Table:**
| Target Frequency | ARR Value |
|------------------|-----------|
| 0.5 Hz | 2000 ms |
| 1.0 Hz | 1000 ms |
| 2.0 Hz | 500 ms  |
| 4.0 Hz | 250 ms  |

### How to Build (Q1)

This project was generated using **STM32CubeIDE**. To view or flash the code:

1. **Import** the `Q1_STM32_Firmware/` folder into STM32CubeIDE
2. **Build** the project (`Project` → `Build Project`)
3. **Connect** the NUCLEO-G070RB board via ST-Link USB
4. **Debug/Run** to flash the firmware and start execution

### Q1 Expected Behavior

- **Initial State**: LED blinks at 0.5 Hz (1 second on, 1 second off)
- **On Each Button Press**: LED frequency increases to the next state in the cycle
- **State Cycle**: 0.5 Hz → 1.0 Hz → 2.0 Hz → 4.0 Hz → 0.5 Hz (repeats)
- **Debounce Window**: Button presses within 50ms of the previous press are ignored
- **Power Efficiency**: MCU sleeps between interrupts; power draw depends on blink frequency

---

## Question 2: Online C Compiler Sensor Simulation

### Q2 Overview

This C program simulates an asynchronous producer-consumer embedded system using POSIX threads. It generates simulated sensor bytes at regular intervals and periodically processes them if the buffer meets a specific threshold, demonstrating real-world embedded systems concurrency challenges.

**Key Features:**
- Thread-safe producer-consumer pattern
- Mutex-protected shared buffer
- Memory-dump hex output format
- Designed for online compiler evaluation

### How to Run (Q2)

Since this is designed for online evaluation with minimal setup:

1. **Copy** the contents of `Q2_Sensor_Simulation/q2_sensor_sim.c`
2. **Paste** into an online compiler supporting POSIX threads (e.g., [Programiz Online C Compiler](https://www.programiz.com/c-programming/online-compiler/))
3. **Click Run** and observe the console output stream with formatted sensor data

### System Architecture

The application utilizes `<pthread.h>` to run two concurrent threads with synchronized access to a shared buffer:

#### Producer Thread
- **Wake Interval**: Every 1 simulated second
- **Action**: Generates 0–5 random bytes and appends them to the shared buffer
- **Thread Safety**: Acquires mutex lock before buffer modification

#### Consumer Thread
- **Wake Interval**: Every 10 simulated seconds
- **Condition**: Processes buffer only if it contains ≥ 50 bytes
- **Action**: Prints the latest 50 bytes in memory-dump hex format and removes them from the buffer
- **Thread Safety**: Acquires mutex lock before buffer read/modification

#### Thread Safety Mechanism
A `pthread_mutex_t` lock completely guards the global buffer structure, preventing race conditions where the producer might write while the consumer is calculating indexes or resetting the byte counter.

**Critical Section Protection:**
```
Mutex Lock
├── Check buffer state
├── Read/Write buffer data
└── Update buffer metadata
Mutex Unlock
```

### Design Assumptions

To map the assignment requirements into a functional, verifiable online simulation, the following assumptions were made:

#### Time Scaling for Online Compilers
Online IDEs often have strict execution time limits (killing processes after ~10–15 seconds). To ensure the reviewer can see multiple 10-second and 20-second processing cycles within a reasonable runtime, time is scaled:
- 1 real second = 1 simulated second (using `usleep()` for sub-second precision)
- This allows the full producer-consumer cycle to execute and demonstrate correctness in ~15–20 seconds

#### Thread Support
The target online compiler operates in a Linux-like sandbox that fully supports `pthread` and `usleep()` system calls, enabling accurate thread scheduling and timing simulations.

#### Buffer Tail Deletion (FIFO vs. LIFO Interpretation)
The assignment states: *"print only the latest 50 bytes... and delete the printed bytes."*

If the buffer has 54 bytes at a 20-second processing mark:
- The **newest/latest 50 bytes** are printed (bytes 5–54)
- The printed bytes are then removed, leaving the oldest 4 bytes (0–4) in the buffer
- This follows a tail-deletion (FIFO-friendly) approach rather than head-deletion

### Q2 Expected Behavior

- **0–10 seconds**: Producer generates bytes; buffer accumulates
- **10-second mark**: If buffer ≥ 50 bytes, consumer prints hex dump and clears printed bytes
- **10–20 seconds**: Cycle repeats; new bytes continue accumulating
- **20-second mark**: Second consumer cycle triggers if threshold met
- **Console Output**: Timestamped lines showing producer byte counts and consumer hex dumps
- **Execution Time**: Complete cycle runs in ~20–25 seconds on typical online compilers

---

## Requirements

### Question 1 (Q1) - STM32 Firmware

- **Hardware**: NUCLEO-G070RB board with STM32G070RB microcontroller
- **Development Tools**: 
  - STM32CubeIDE (latest stable version)
  - ST-Link USB debugger/programmer (included with Nucleo board)
  - ARM GCC toolchain (bundled with STM32CubeIDE)
- **Knowledge**: C, embedded systems, interrupt handlers, timer configuration

### Question 2 (Q2) - Online C Compiler Simulation

- **Online Compiler**: Any C compiler with POSIX pthread support
  - **Recommended**: [Programiz Online C Compiler](https://www.programiz.com/c-programming/online-compiler/)
  - **Alternatives**: Replit, OnlineGDB, or any Linux-based online IDE
- **Libraries**: `<pthread.h>`, `<stdio.h>`, `<stdlib.h>`, `<unistd.h>`, `<time.h>`
- **System**: Linux-like environment (automatically provided by online compilers)

---

## Project Structure

```
NoshRoboticsAssignment/
├── README.md                          # This file
├── Q1_STM32_Firmware/                 # STM32CubeIDE project
│   ├── Core/
│   │   ├── Src/
│   │   │   ├── main.c
│   │   │   ├── stm32g0xx_it.c        # Interrupt handlers
│   │   │   └── ...
│   │   └── Inc/
│   │       └── ...
│   ├── Drivers/
│   └── ...
└── Q2_Sensor_Simulation/              # Online compiler project
    ├── q2_sensor_sim.c               # Single-file producer-consumer
    └── README.md                       # Q2-specific documentation
```

---

## Summary

This assignment demonstrates:

✓ **Low-power firmware design** with sleep modes and dynamic peripheral reconfiguration  
✓ **Real-time interrupt handling** with debouncing and non-blocking timing  
✓ **Thread-safe concurrent programming** using mutex synchronization  
✓ **Embedded systems best practices** for performance and reliability  

Both solutions are production-ready and follow industry best practices for embedded systems development.
