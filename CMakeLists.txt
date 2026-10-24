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

# Add Unity testing framework
add_subdirectory(extern/unity EXCLUDE_FROM_ALL)

# Add Raylib and Raygui
set(BUILD_EXAMPLES OFF CACHE BOOL "" FORCE) # Don't build raylib examples
add_subdirectory(extern/raylib EXCLUDE_FROM_ALL)

# Core emulator library 
file(GLOB_RECURSE CORE_SOURCES CONFIGURE_DEPENDS "src/risc/*.c")
add_library(risc_core STATIC ${CORE_SOURCES})
target_include_directories(risc_core PUBLIC src)

# Tools emulator library 
file(GLOB_RECURSE TOOL_SOURCES CONFIGURE_DEPENDS "src/tools/*.c")
add_library(risc_tools STATIC ${TOOL_SOURCES})
target_include_directories(risc_tools PUBLIC src)

target_link_libraries(risc_core PUBLIC risc_tools)

# GUI library 
file(GLOB_RECURSE GUI_SOURCES CONFIGURE_DEPENDS "src/gui/*.c")
add_library(risc_gui STATIC ${GUI_SOURCES})
target_include_directories(risc_gui PUBLIC src src/gui extern/raygui/src)
target_link_libraries(risc_gui PUBLIC raylib risc_core risc_tools)

# Main executable
add_executable(TakeTheRisc src/main.c)
target_link_libraries(TakeTheRisc PRIVATE risc_core risc_tools risc_gui)

# Enable CTest
enable_testing()

# Copy relevant RISC-V test files into the build directory if present
file(GLOB RISCV_TEST_FILES CONFIGURE_DEPENDS 
    "${CMAKE_CURRENT_SOURCE_DIR}/tests/riscv-tests/rv32ui-p-*"
    "${CMAKE_CURRENT_SOURCE_DIR}/tests/riscv-tests/rv32um-p-*"
    "${CMAKE_CURRENT_SOURCE_DIR}/tests/riscv-tests/rv32mi-p-*"
    "${CMAKE_CURRENT_SOURCE_DIR}/tests/riscv-tests/rv32ua-p-*"
)

# Automatically discover and register all tests in tests/
file(GLOB_RECURSE TEST_SOURCES CONFIGURE_DEPENDS "tests/*.c")

# If no RISC-V test binaries exist (e.g. in a fresh git clone), skip the riscv-tests runner
if(NOT RISCV_TEST_FILES)
    list(FILTER TEST_SOURCES EXCLUDE REGEX "test_cpu_riscv_tests\\.c$")
    message(STATUS "RISC-V compliance test files not found; skipping test_cpu_riscv_tests")
endif()

foreach(TEST_SOURCE ${TEST_SOURCES})
    get_filename_component(TEST_NAME ${TEST_SOURCE} NAME_WE)

    add_executable(${TEST_NAME} ${TEST_SOURCE})
    target_link_libraries(${TEST_NAME} PRIVATE risc_core risc_tools unity)

    add_test(NAME ${TEST_NAME} COMMAND ${TEST_NAME})
    set_tests_properties(${TEST_NAME} PROPERTIES WORKING_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}")
endforeach()

if(RISCV_TEST_FILES)
    add_custom_target(copy_test_data ALL
        COMMAND ${CMAKE_COMMAND} -E make_directory "${CMAKE_CURRENT_BINARY_DIR}/tests/riscv-tests"
        COMMAND ${CMAKE_COMMAND} -E copy_if_different ${RISCV_TEST_FILES} "${CMAKE_CURRENT_BINARY_DIR}/tests/riscv-tests"
        COMMENT "Copying RISC-V test files into build directory"
    )
endif()
