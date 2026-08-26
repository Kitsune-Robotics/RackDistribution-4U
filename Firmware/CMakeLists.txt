cmake_minimum_required(VERSION 3.13)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# pico-sdk from git if PICO_SDK_PATH isn't set
set(PICO_SDK_FETCH_FROM_GIT on)
include(pico_sdk_import.cmake)

project(rack_distribution)

set(PICO_STDIO_USB 1)
set(PICO_STDIO_UART 0)

pico_sdk_init()

# -----------------
# FreeRTOS (kernel)
# -----------------

set(FREERTOS_KERNEL_PATH ${CMAKE_CURRENT_LIST_DIR}/lib/FreeRTOS-Kernel)

add_library(freertos_kernel STATIC
        ${FREERTOS_KERNEL_PATH}/event_groups.c
        ${FREERTOS_KERNEL_PATH}/list.c
        ${FREERTOS_KERNEL_PATH}/queue.c
        ${FREERTOS_KERNEL_PATH}/stream_buffer.c
        ${FREERTOS_KERNEL_PATH}/tasks.c
        ${FREERTOS_KERNEL_PATH}/timers.c
        ${FREERTOS_KERNEL_PATH}/portable/GCC/ARM_CM0/port.c
        ${FREERTOS_KERNEL_PATH}/portable/GCC/ARM_CM0/portasm.c
        ${FREERTOS_KERNEL_PATH}/portable/MemMang/heap_4.c
)

target_include_directories(freertos_kernel PUBLIC
        ${FREERTOS_KERNEL_PATH}/include
        ${FREERTOS_KERNEL_PATH}/portable/GCC/ARM_CM0
        ${CMAKE_CURRENT_LIST_DIR}/include
)

# ----------
# Firmware
# ----------

add_executable(rack_distribution
        main.c
        lib/analog_task.c
        lib/freertos_support.c
        lib/indicators.c
        lib/neopixel_ws2812/neopixel_ws2812.c
)

pico_generate_pio_header(rack_distribution
        ${CMAKE_CURRENT_LIST_DIR}/lib/neopixel_ws2812/ws2812.pio)

target_include_directories(rack_distribution PRIVATE
        ${CMAKE_CURRENT_LIST_DIR}/include
        ${CMAKE_CURRENT_LIST_DIR}/lib/neopixel_ws2812
)

target_link_libraries(rack_distribution
        pico_stdlib
        hardware_pio
        hardware_clocks
        hardware_adc
        m
        freertos_kernel
)

pico_add_extra_outputs(rack_distribution)
