cmake_minimum_required(VERSION 3.0)
if ("${CMAKE_C_COMPILER}" STREQUAL "")
  set(CMAKE_C_COMPILER clang)
endif()
set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED ON)
set(CMAKE_C_EXTENSIONS OFF)
project(base C)
enable_testing()

file(GLOB sources "*.c" "*.h")
find_program(CLANG_FORMAT_EXE clang-format)
add_custom_target(${PROJECT_NAME}-format ALL
  COMMAND ${CLANG_FORMAT_EXE} --style="{BasedOnStyle: llvm, ColumnLimit: 120, BinPackArguments: false, BinPackParameters: false}" -i ${sources}
)

add_executable(test_base base_test.c)
list(APPEND tests test_base)

foreach(target ${tests})
  add_test(NAME ${target} COMMAND ${target})
  target_compile_definitions(${target} PRIVATE
    $<$<BOOL:${WIN32}>:__STDC_NO_THREADS__>
    $<$<BOOL:${WIN32}>:_WIN32_WINNT=0x0502>
    $<$<BOOL:${WIN32}>:_CONSOLE>
    USE_STR
    USE_WSTR
    LEAK_DETECTOR
    ALLOCATE_LOGGER
  )
  target_compile_options(${target} PRIVATE
    $<$<BOOL:${WIN32}>:--rtlib=compiler-rt>
    -mstackrealign
    -Wall
    -Wextra
    -Werror=return-type
    -pedantic-errors
    -O0
  )
  target_link_options(${target} PRIVATE
    $<$<BOOL:${WIN32}>:--rtlib=compiler-rt>
    $<$<BOOL:${WIN32}>:-Wl,--kill-at>
    $<$<BOOL:${WIN32}>:-no-pthread>
    -static
  )
  target_link_libraries(${target} PRIVATE
    $<$<NOT:$<BOOL:${WIN32}>>:pthread>
  )
endforeach(target)
