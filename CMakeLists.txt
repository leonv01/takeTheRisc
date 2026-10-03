cmake_minimum_required(VERSION 3.26)
project(TakeTheRisc C)

# Set the C standard
set(CMAKE_C_STANDARD 99)
set(CMAKE_C_STANDARD_REQUIRED True)


if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Debug CACHE STRING "Build type" FORCE)
endif()

if(CMAKE_BUILD_TYPE STREQUAL "Debug")
    set(CMAKE_C_FLAGS_DEBUG "${CMAKE_C_FLAGS_DEBUG} -g")
endif()

# Include necessary directories
include_directories(src)
#include_directories(extern/unity/src)
add_subdirectory(extern/unity EXCLUDE_FROM_ALL)

# Enable CTest
enable_testing()

file(GLOB_RECURSE APP_SOURCES
    src/*.c
)

add_executable(TakeTheRisc ${APP_SOURCES})

add_executable(test_memory_read
    tests/memory/test_memory_read.c
    src/risc/risc-memory.c
)

add_executable(test_memory_write
    tests/memory/test_memory_write.c
    src/risc/risc-memory.c
)

target_include_directories(test_memory_read PRIVATE src)
target_include_directories(test_memory_write PRIVATE src)
target_link_libraries(test_memory_read PRIVATE unity)
target_link_libraries(test_memory_write PRIVATE unity)

add_test(NAME test_memory_read COMMAND test_memory_read)
add_test(NAME test_memory_write COMMAND test_memory_read)