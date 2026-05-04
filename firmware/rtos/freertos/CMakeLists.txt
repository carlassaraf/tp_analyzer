cmake_minimum_required(VERSION 3.12)

set(CORE "RP2350_ARM_NTZ")

# Add FreeRTOS source files
add_library(freertos STATIC
    event_groups.c
    list.c
    queue.c
    stream_buffer.c
    tasks.c
    timers.c
    portable/GCC/${CORE}/port.c
    portable/MemMang/heap_4.c
    portable/GCC/${CORE}/mpu_wrappers_v2_asm.c
    portable/GCC/${CORE}/portasm.c
)

# Include FreeRTOS header files
target_include_directories(freertos PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_CURRENT_SOURCE_DIR}/include
    ${CMAKE_CURRENT_SOURCE_DIR}/portable/GCC/${CORE}/include
)

# Add dependencies
target_link_libraries(freertos PUBLIC pico_stdlib hardware_exception)