#include "FreeRTOS.h"
#include "task.h"

#include "pico/stdlib.h"

// Pico SDK vectors are isr_*, FreeRTOS wants the CMSIS names. Jump over.
void SVC_Handler(void);
void PendSV_Handler(void);
void SysTick_Handler(void);

void __attribute__((naked)) isr_svcall(void) {
  __asm volatile("ldr r0, =SVC_Handler\n"
                 "bx  r0\n");
}
void __attribute__((naked)) isr_pendsv(void) {
  __asm volatile("ldr r0, =PendSV_Handler\n"
                 "bx  r0\n");
}
void __attribute__((naked)) isr_systick(void) {
  __asm volatile("ldr r0, =SysTick_Handler\n"
                 "bx  r0\n");
}

static void fatal_blink(uint32_t on_ms, uint32_t off_ms) {
  gpio_init(PICO_DEFAULT_LED_PIN);
  gpio_set_dir(PICO_DEFAULT_LED_PIN, GPIO_OUT);

  while (true) {
    gpio_put(PICO_DEFAULT_LED_PIN, 1);
    busy_wait_ms(on_ms);
    gpio_put(PICO_DEFAULT_LED_PIN, 0);
    busy_wait_ms(off_ms);
  }
}

void vAssertCalled(const char *file, int line) {
  (void)file;
  (void)line;
  fatal_blink(100, 100);
}

void vApplicationMallocFailedHook(void) { fatal_blink(80, 400); }

void vApplicationStackOverflowHook(TaskHandle_t xTask, char *pcTaskName) {
  (void)xTask;
  (void)pcTaskName;
  fatal_blink(600, 200);
}
