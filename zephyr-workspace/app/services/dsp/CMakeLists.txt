add_library(dsp STATIC dsp.c)
target_include_directories(dsp PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
# NOT linking a "CMSISDSP" target by name deliberately — same trap as the
# LVGL "lvgl" vs the real "modules__lvgl" target name earlier this
# migration: zephyr_library() with no name derives it from the module's
# path relative to ZEPHYR_BASE (modules/cmsis-dsp -> modules__cmsis-dsp),
# so "CMSISDSP" is very likely not the real target. Doesn't matter: it's a
# kernel-mode Zephyr library either way, so it auto-registers into
# ZEPHYR_LIBS and is already unconditionally part of the final link —
# nothing here needs to reference it by name for symbols to resolve.
# What *is* needed is zephyr_interface, since CMSIS-DSP's own CMakeLists.txt
# puts arm_math.h's include path on that global target (zephyr_include_
# directories(), not zephyr_library_include_directories() — same mechanism
# LVGL used), which is what app_lib/ui both needed this exact fix for too.
target_link_libraries(dsp PUBLIC zephyr_interface)
add_dependencies(dsp zephyr_generated_headers)