# NoshRoboticsAssignment
Embedded Systems Assignment for Internship at Nosh Robotics



Candidate: Shreesh Purandarvittal Tadas

This repository contains the solutions for the embedded systems engineering assignment for Nosh Robotics. It is divided into two parts: a bare-metal hardware state machine targeted for the STM32G070RB, and a multithreaded sensor data simulation designed for online C compilers.



Question 1: STM32G070RB Switch & LED State Machine

Overview

This project implements a low-power, interrupt-driven state machine on the NUCLEO-G070RB board. A single tactile switch cycles through four distinct LED blinking frequencies (0.5Hz, 1.0Hz, 2.0Hz, and OFF), rolling over on the fifth press.

Hardware Mapping

(Note: This firmware is designed to utilize the existing on-board peripherals of the NUCLEO-G070RB, requiring no external breadboard circuitry).

User Button (PC13): Utilizing the on-board blue tactile switch. Configured as an External Interrupt (EXTI) triggering on the falling edge (active-low).

User LED (PA5): Utilizing the on-board green LED. Configured as a standard GPIO Push-Pull output.

Timebase (TIM6): Configured with a prescaler to generate a 1ms tick base.

Firmware Architecture & Power Optimization

To strictly adhere to the power optimization requirement, this firmware avoids blocking loops and polling entirely.

Sleep Mode Implementation: Instead of spinning in a while(1) loop, the MCU executes HAL_PWR_EnterSLEEPMode(PWR_MAINREGULATOR_ON, PWR_SLEEPENTRY_WFI);. This halts the ARM Cortex core clock while keeping the peripherals (EXTI, TIM6) alive.

SysTick Management: Before entering Sleep Mode, HAL_SuspendTick() is called so the 1ms SysTick interrupt doesn't unnecessarily wake the core. HAL_ResumeTick() is immediately called upon waking (inside the EXTI callback) to restore timekeeping.

Non-Blocking Debounce: Inside the EXTI callback, I implemented a 50ms software debounce using HAL_GetTick(). Unlike HAL_Delay(), this is non-blocking and prevents ISR deadlocks if interrupt priorities overlap.

Dynamic Timer Reconfiguration: Rather than running multiple timers, the Auto-Reload Register (ARR) of TIM6 is dynamically updated on each button press. I also explicitly reset the timer counter (__HAL_TIM_SET_COUNTER) during a state change to prevent overflow delays when switching from a slow to a fast frequency.

How to Build

This project was generated using STM32CubeIDE. To view or flash the code:

Import the Q1_STM32_Firmware folder into STM32CubeIDE.

Build the project (Project -> Build Project).

Connect the NUCLEO-G070RB and click Debug/Run to flash the board for the physical showcase.

Question 2: Online C Compiler Sensor Simulation

Overview

This C program simulates an asynchronous producer-consumer embedded system. It generates simulated sensor bytes every second and periodically processes them if the buffer meets a specific threshold.

How to Run

Since this is designed for online evaluation:

Copy the contents of Q2_Sensor_Simulation/q2_sensor_sim.c.

Paste it into an online compiler that supports standard POSIX threads, such as Programiz Online C Compiler.

Click Run. The console will stream the formatted output.

System Architecture

The application utilizes <pthread.h> to run two concurrent threads:

Producer Thread: Wakes up every 1 simulated second, generates 0-5 random bytes, and pushes them to a globally accessible structure.

Consumer Thread: Wakes up every 10 simulated seconds. If the buffer contains $\ge$ 50 bytes, it prints the latest 50 bytes in a memory-dump hex format and deletes them.

Thread Safety: A pthread_mutex_t lock completely guards the global struct, preventing race conditions where the producer might write while the consumer is calculating indexes or resetting the counter.

Design Assumptions

To map the assignment requirements into a functional, verifiable online simulation, the following assumptions were made:

Time Scaling for Online Compilers: Online IDEs often have strict execution time limits (killing processes after ~10-15 seconds). To ensure the reviewer can see multiple 10s and 20s processing cycles without staring at a slow console, time is scaled 5x faster. (1 simulated second = 200ms of real execution time).

Thread Support: The target online compiler operates in a Linux-like sandbox that supports pthread and usleep.

Buffer Tail Deletion: The prompt states to "print only the latest 50 bytes... and delete the printed bytes". If the buffer has 54 bytes at the 20-second mark, the newest 50 are printed, and the buffer count is decremented by 50. The 4 oldest bytes are intentionally left untouched in memory, waiting to be overwritten or pushed forward.
