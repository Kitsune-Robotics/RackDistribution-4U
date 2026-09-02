#pragma once

// RP2040

#include <stddef.h>
#include <stdint.h>

void vAssertCalled(const char *file, int line);

#define configUSE_PREEMPTION 1
#define configUSE_TIME_SLICING 1
#define configUSE_TICKLESS_IDLE 0

#define configCPU_CLOCK_HZ ((unsigned long)125000000UL)
#define configTICK_RATE_HZ ((TickType_t)1000)
#define configTICK_TYPE_WIDTH_IN_BITS TICK_TYPE_WIDTH_32_BITS

#define configMAX_PRIORITIES 5
#define configMINIMAL_STACK_SIZE ((unsigned short)256)
#define configMAX_TASK_NAME_LEN 16

#define configSUPPORT_STATIC_ALLOCATION 0
#define configSUPPORT_DYNAMIC_ALLOCATION 1
#define configTOTAL_HEAP_SIZE ((size_t)(32 * 1024))

#define configUSE_MUTEXES 1
#define configUSE_RECURSIVE_MUTEXES 0
#define configUSE_COUNTING_SEMAPHORES 0
#define configUSE_TIMERS 0
#define configUSE_QUEUE_SETS 0
#define configQUEUE_REGISTRY_SIZE 0
#define configUSE_TASK_NOTIFICATIONS 1
#define configTASK_NOTIFICATION_ARRAY_ENTRIES 1

#define configUSE_IDLE_HOOK 0
#define configUSE_TICK_HOOK 0
#define configUSE_MALLOC_FAILED_HOOK 1
#define configCHECK_FOR_STACK_OVERFLOW 2
#define configUSE_TRACE_FACILITY 0
#define configGENERATE_RUN_TIME_STATS 0

#define configASSERT(x)                                                          \
  if ((x) == 0) {                                                                \
    portDISABLE_INTERRUPTS();                                                    \
    vAssertCalled(__FILE__, __LINE__);                                           \
  }

#define configENABLE_MPU 0

// Pico SDK uses isr_* names. Wrappers are in freertos_support.c.
#define configCHECK_HANDLER_INSTALLATION 0

#define INCLUDE_vTaskPrioritySet 0
#define INCLUDE_uxTaskPriorityGet 0
#define INCLUDE_vTaskDelete 0
#define INCLUDE_vTaskSuspend 1
#define INCLUDE_xTaskDelayUntil 1
#define INCLUDE_vTaskDelay 1
#define INCLUDE_xTaskGetSchedulerState 0
#define INCLUDE_xTaskGetCurrentTaskHandle 1
#define INCLUDE_vTaskCleanUpResources 0
#define INCLUDE_xTaskGetIdleTaskHandle 0
#define INCLUDE_eTaskGetState 0
#define INCLUDE_uxTaskGetStackHighWaterMark 0
#define INCLUDE_xTaskAbortDelay 0
#define INCLUDE_xTaskGetHandle 0
