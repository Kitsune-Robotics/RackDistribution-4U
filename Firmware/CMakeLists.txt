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
        states/states.c
        states/state_init.c
        states/state_standby.c
        states/state_run.c
        states/state_cooldown.c
        aquacomputer/usb_descriptors.c
        aquacomputer/aquacomputer.c
        aquacomputer/msc_uf2.c
)

pico_generate_pio_header(rack_distribution
        ${CMAKE_CURRENT_LIST_DIR}/lib/neopixel_ws2812/ws2812.pio)

target_include_directories(rack_distribution PRIVATE
        ${CMAKE_CURRENT_LIST_DIR}/aquacomputer
        ${CMAKE_CURRENT_LIST_DIR}/include
        ${CMAKE_CURRENT_LIST_DIR}/lib/neopixel_ws2812
        ${CMAKE_CURRENT_LIST_DIR}/states
)

# pico_usb_reset needs the reset interface # before usb_reset_config.h
target_compile_options(rack_distribution PRIVATE
        "SHELL:-include ${CMAKE_CURRENT_LIST_DIR}/include/usb_reset_itf.h"
)

target_compile_definitions(rack_distribution PRIVATE
        PICO_STDIO_USB_USE_DEFAULT_DESCRIPTORS=0
        PICO_STDIO_USB_ENABLE_TINYUSB_INIT=1
        PICO_STDIO_USB_ENABLE_IRQ_BACKGROUND_TASK=1
        PICO_ENABLE_USB_RESET_VIA_VENDOR_INTERFACE=1
        PICO_ENABLE_USB_RESET_VIA_BAUD_RATE=1
        PICO_USB_RESET_SUPPORT_MS_OS_20_DESCRIPTOR=1
        PICO_USB_RESET_INCLUDE_DEFAULT_APP_DRIVER_CB=1
)

target_link_libraries(rack_distribution
        pico_stdlib
        tinyusb_device
        pico_unique_id
        hardware_pio
        hardware_clocks
        hardware_adc
        hardware_flash
        hardware_sync
        m
        freertos_kernel
)

pico_add_extra_outputs(rack_distribution)
