/*
    FreeRTOS V9.0.0 - Copyright (C) 2016 Real Time Engineers Ltd.
    All rights reserved

    VISIT http://www.FreeRTOS.org TO ENSURE YOU ARE USING THE LATEST VERSION.

    This file is part of the FreeRTOS distribution.

    FreeRTOS is free software; you can redistribute it and/or modify it under
    the terms of the GNU General Public License (version 2) as published by the
    Free Software Foundation >>>> AND MODIFIED BY <<<< the FreeRTOS exception.

    ***************************************************************************
    >>!   NOTE: The modification to the GPL is included to allow you to     !<<
    >>!   distribute a combined work that includes FreeRTOS without being   !<<
    >>!   obliged to provide the source code for proprietary components     !<<
    >>!   outside of the FreeRTOS kernel.                                   !<<
    ***************************************************************************

    FreeRTOS is distributed in the hope that it will be useful, but WITHOUT ANY
    WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
    FOR A PARTICULAR PURPOSE.  Full license text is available on the following
    link: http://www.freertos.org/a00114.html

    ***************************************************************************
     *                                                                       *
     *    FreeRTOS provides completely free yet professionally developed,    *
     *    robust, strictly quality controlled, supported, and cross          *
     *    platform software that is more than just the market leader, it     *
     *    is the industry's de facto standard.                               *
     *                                                                       *
     *    Help yourself get started quickly while simultaneously helping     *
     *    to support the FreeRTOS project by purchasing a FreeRTOS           *
     *    tutorial book, reference manual, or both:                          *
     *    http://www.FreeRTOS.org/Documentation                              *
     *                                                                       *
    ***************************************************************************

    http://www.FreeRTOS.org/FAQHelp.html - Having a problem?  Start by reading
    the FAQ page "My application does not run, what could be wrong?".  Have you
    defined configASSERT()?

    http://www.FreeRTOS.org/support - In return for receiving this top quality
    embedded software for free we request you assist our global community by
    participating in the support forum.

    http://www.FreeRTOS.org/training - Investing in training allows your team to
    be as productive as possible as early as possible.  Now you can receive
    FreeRTOS training directly from Richard Barry, CEO of Real Time Engineers
    Ltd, and the world's leading authority on the world's leading RTOS.

    http://www.FreeRTOS.org/plus - A selection of FreeRTOS ecosystem products,
    including FreeRTOS+Trace - an indispensable productivity tool, a DOS
    compatible FAT file system, and our tiny thread aware UDP/IP stack.

    http://www.FreeRTOS.org/labs - Where new FreeRTOS products go to incubate.
    Come and try FreeRTOS+TCP, our new open source TCP/IP stack for FreeRTOS.

    http://www.OpenRTOS.com - Real Time Engineers ltd. license FreeRTOS to High
    Integrity Systems ltd. to sell under the OpenRTOS brand.  Low cost OpenRTOS
    licenses offer ticketed support, indemnification and commercial middleware.

    http://www.SafeRTOS.com - High Integrity Systems also provide a safety
    engineered and independently SIL3 certified version for use in safety and
    mission critical applications that require provable dependability.

    1 tab == 4 spaces!
*/

#ifndef FREERTOS_CONFIG_H
#define FREERTOS_CONFIG_H

#include "sys.h"
#include "usart.h"

/* Include different stdint.h files based on compiler being used */
#if defined(__ICCARM__) || defined(__CC_ARM) || defined(__GNUC__)
#include <stdint.h>
extern uint32_t SystemCoreClock;
#endif

#define vAssertCalled(char, int) printf("Error:%s,%d\r\n", char, int)
#define configASSERT(x) \
    if ((x) == 0)       \
    vAssertCalled(__FILE__, __LINE__)

/***************************************************************************************************************/
/*                                        FreeRTOS Basic Configuration                                             */
/***************************************************************************************************************/
#define configUSE_PREEMPTION 1                    // 1 to use preemptive scheduling, 0 to use cooperative scheduling
#define configUSE_TIME_SLICING 1                  // 1 to use time slicing, 0 to use a single processor
#define configUSE_PORT_OPTIMISED_TASK_SELECTION 1 // 1 to use port optimised task selection
                                                  // MCU doesn't have these hardware features which must be implemented in the port

#define configUSE_TICKLESS_IDLE 0                      // 1 to use tickless idle mode, 0 to use normal idle mode
#define configUSE_QUEUE_SETS 1                         // 1 to use queue sets, 0 to not use queue sets
#define configCPU_CLOCK_HZ (SystemCoreClock)           // CPU frequency HZ
#define configTICK_RATE_HZ (1000)                      // Tick rate HZ, 1000 means 1ms
#define configMAX_PRIORITIES (32)                      // Maximum number of priorities
#define configMINIMAL_STACK_SIZE ((unsigned short)130) // Minimal stack size
#define configMAX_TASK_NAME_LEN (16)                   // Maximum length of task names

#define configUSE_16_BIT_TICKS 0 // 1 to use 16-bit ticks, 0 to use 32-bit ticks (no quote)

#define configIDLE_SHOULD_YIELD 1        // 1 to yield CPU to other tasks of the same priority, 0 to not yield
#define configUSE_TASK_NOTIFICATIONS 1   // 1 to use task notifications, 0 to not use task notifications
#define configUSE_MUTEXES 1              // 1 to use mutexes, 0 to not use mutexes
#define configQUEUE_REGISTRY_SIZE 8      // 0 to disable queue registry, 8 to enable queue registry
#define configCHECK_FOR_STACK_OVERFLOW 0 // 0 to disable stack overflow checking, 1 to enable stack overflow checking
                                         // The user provides a hook function that is called if a stack overflow is detected
#define configUSE_RECURSIVE_MUTEXES 1    // 1 to use recursive mutexes, 0 to not use recursive mutexes
#define configUSE_MALLOC_FAILED_HOOK 0   // 1 to use malloc failed hook, 0 to not use malloc failed hook
#define configUSE_APPLICATION_TASK_TAG 0
#define configUSE_COUNTING_SEMAPHORES 1 // 1 to use counting semaphores, 0 to not use counting semaphores

/***************************************************************************************************************/
/*                                FreeRTOS Memory Allocation                                               */
/***************************************************************************************************************/
#define configSUPPORT_DYNAMIC_ALLOCATION 1
#define configTOTAL_HEAP_SIZE ((size_t)(25 * 1024)) // Total heap size

/***************************************************************************************************************/
/*                                FreeRTOS Hooks                                                        */
/***************************************************************************************************************/
#define configUSE_IDLE_HOOK 0 // 1 to use idle hook, 0 to not use
#define configUSE_TICK_HOOK 0 // 1 to use tick hook, 0 to not use

/***************************************************************************************************************/
/*                                FreeRTOS Runtime Statistics                                               */
/***************************************************************************************************************/
#define configGENERATE_RUN_TIME_STATS 0        // 1 to generate run time statistics
#define configUSE_TRACE_FACILITY 1             // 1 to use trace facility
#define configUSE_STATS_FORMATTING_FUNCTIONS 1 // 1 to use formatting functions
// prvWriteNameToBuffer(),vTaskList(),
// vTaskGetRunTimeStats()

/***************************************************************************************************************/
/*                                FreeRTOS Co-routines                                                    */
/***************************************************************************************************************/
#define configUSE_CO_ROUTINES 0             // 1 to use co-routines, 0 to not use
#define configMAX_CO_ROUTINE_PRIORITIES (2) // Co-routine priorities

/***************************************************************************************************************/
/*                                FreeRTOS Timers                                                        */
/***************************************************************************************************************/
#define configUSE_TIMERS 1                                          // 1 to use timers, 0 to not use
#define configTIMER_TASK_PRIORITY (configMAX_PRIORITIES - 1)        // Timer task priority
#define configTIMER_QUEUE_LENGTH 5                                  // Timer queue length
#define configTIMER_TASK_STACK_DEPTH (configMINIMAL_STACK_SIZE * 2) // Timer task stack depth

/***************************************************************************************************************/
/*                                FreeRTOS Selected Features                                                */
/***************************************************************************************************************/
#define INCLUDE_xTaskGetSchedulerState 1
#define INCLUDE_vTaskPrioritySet 1
#define INCLUDE_uxTaskPriorityGet 1
#define INCLUDE_vTaskDelete 1
#define INCLUDE_vTaskCleanUpResources 1
#define INCLUDE_vTaskSuspend 1
#define INCLUDE_vTaskDelayUntil 1
#define INCLUDE_vTaskDelay 1
#define INCLUDE_eTaskGetState 1
#define INCLUDE_xTimerPendFunctionCall 1

/***************************************************************************************************************/
/*                                FreeRTOS Interrupt Configuration                                              */
/***************************************************************************************************************/
#ifdef __NVIC_PRIO_BITS
#define configPRIO_BITS __NVIC_PRIO_BITS
#else
#define configPRIO_BITS 4
#endif

#define configLIBRARY_LOWEST_INTERRUPT_PRIORITY 15     // Lowest interrupt priority
#define configLIBRARY_MAX_SYSCALL_INTERRUPT_PRIORITY 5 // System call interrupt priority
#define configKERNEL_INTERRUPT_PRIORITY (configLIBRARY_LOWEST_INTERRUPT_PRIORITY << (8 - configPRIO_BITS))
#define configMAX_SYSCALL_INTERRUPT_PRIORITY (configLIBRARY_MAX_SYSCALL_INTERRUPT_PRIORITY << (8 - configPRIO_BITS))

/***************************************************************************************************************/
/*                                FreeRTOS Interrupt Handlers                                                */
/***************************************************************************************************************/
#define xPortPendSVHandler PendSV_Handler
#define vPortSVCHandler SVC_Handler

#endif /* FREERTOS_CONFIG_H */
