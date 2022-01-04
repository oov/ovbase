option(USE_STR "use struct str" ON)
option(USE_WSTR "use struct wstr" ON)
option(LEAK_DETECTOR "use leak detector" ON)
option(ALLOCATE_LOGGER "use allocate logger" ON)
option(USE_COMPILER_RT "use compiler-rt runtime" ON)

file(GLOB sources "*.c" "../include/*.h")
find_program(CLANG_FORMAT_EXE clang-format)
add_custom_target(${PROJECT_NAME}-format ALL
  COMMAND ${CLANG_FORMAT_EXE} -style=file -i ${sources}
)

add_compile_definitions(
  $<$<BOOL:${WIN32}>:__STDC_NO_THREADS__>
  $<$<BOOL:${WIN32}>:_WIN32_WINNT=0x0502>
  $<$<BOOL:${USE_COMPILER_RT}>:USE_COMPILER_RT>
  $<$<BOOL:${USE_STR}>:USE_STR>
  $<$<BOOL:${USE_WSTR}>:USE_WSTR>
  $<$<BOOL:${LEAK_DETECTOR}>:LEAK_DETECTOR>
  $<$<BOOL:${ALLOCATE_LOGGER}>:ALLOCATE_LOGGER>
  $<$<CONFIG:Release>:NDEBUG>
)
add_compile_options(
  $<$<AND:$<BOOL:${WIN32}>,$<BOOL:${USE_COMPILER_RT}>>:--rtlib=compiler-rt>
  -mstackrealign
  -Wall
  -Wextra
  -Werror
  -Weverything
  -Wshadow
  -Werror=return-type
  -pedantic-errors
  -ffunction-sections
  -fdata-sections
  $<$<CONFIG:Debug>:-O0>
  $<$<CONFIG:Release>:-O2>
)
add_link_options(
  -Wl,--gc-sections
  # -Wl,--print-gc-sections
  $<$<AND:$<BOOL:${WIN32}>,$<BOOL:${USE_COMPILER_RT}>>:--rtlib=compiler-rt>
  $<$<BOOL:${WIN32}>:-no-pthread>
  -static
)
link_libraries(
  $<$<NOT:$<BOOL:${WIN32}>>:pthread>
)

add_library(base STATIC base.c)

add_executable(test_base base_test.c)
list(APPEND tests test_base)

foreach(target ${tests})
  add_test(NAME ${target} COMMAND ${target})
  target_compile_definitions(${target} PRIVATE
    LEAK_DETECTOR
    ALLOCATE_LOGGER
  )
endforeach(target)
