add_library(dsp STATIC dsp.c)
target_include_directories(dsp PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
target_link_libraries(dsp PUBLIC CMSISDSP)