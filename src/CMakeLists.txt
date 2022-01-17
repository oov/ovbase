option(USE_STR "use struct str" ON)
option(USE_WSTR "use struct wstr" ON)
option(LEAK_DETECTOR "use leak detector" ON)
option(ALLOCATE_LOGGER "use allocate logger" ON)
option(USE_COMPILER_RT "use compiler-rt runtime" ON)
option(SKIP_OVTHREADS_TEST "skip ovthreads tests" ON)

add_library(ovbase STATIC
  array.c
  error.c
  error_en.c
  error_jp.c
  hashmap.c
  hmap/clear.c
  hmap/count.c
  hmap/delete.c
  hmap/free.c
  hmap/get.c
  hmap/new_dynamic.c
  hmap/new_static.c
  hmap/scan.c
  hmap/set.c
  ovbase.c
  ovthreads.c
  str/cat_.c
  str/cat_m_.c
  str/cpy_.c
  str/cpy_m_.c
  str/ncat_.c
  str/ncpy_.c
  str/replace_all_.c
  str/str_.c
  wstr/cat_.c
  wstr/cat_m_.c
  wstr/cpy_.c
  wstr/cpy_m_.c
  wstr/ncat_.c
  wstr/ncpy_.c
  wstr/replace_all_.c
  wstr/str_.c
)
list(APPEND targets ovbase)

add_executable(test_array array_test.c)
list(APPEND targets test_array)
add_executable(test_ovbase ovbase_test.c)
list(APPEND targets test_ovbase)
add_executable(test_hmap hmap/test.c)
list(APPEND targets test_hmap)
add_executable(test_str str/test.c)
list(APPEND targets test_str)
add_executable(test_wstr wstr/test.c)
list(APPEND targets test_wstr)

if(NOT SKIP_OVTHREADS_TEST)
  add_executable(test_ovthteads ovthreads_test.c)
  list(APPEND targets test_ovthteads)
endif()

foreach(target ${targets})
  if(target MATCHES "^test_")
    add_test(NAME ${target} COMMAND ${target})
  endif()
  target_include_directories(${target} PUBLIC
    "${PROJECT_SOURCE_DIR}/include"
  )
  target_compile_definitions(${target} PUBLIC
    $<$<BOOL:${WIN32}>:__STDC_NO_THREADS__>
    $<$<BOOL:${USE_COMPILER_RT}>:USE_COMPILER_RT>
    $<$<BOOL:${USE_STR}>:USE_STR>
    $<$<BOOL:${USE_WSTR}>:USE_WSTR>
    $<$<BOOL:${LEAK_DETECTOR}>:LEAK_DETECTOR>
    $<$<BOOL:${ALLOCATE_LOGGER}>:ALLOCATE_LOGGER>
  )
  target_compile_definitions(${target} PRIVATE
    $<$<BOOL:${WIN32}>:_WIN32_WINNT=0x0502>
    $<$<CONFIG:Release>:NDEBUG>
  )
  target_compile_options(${target} PUBLIC
    $<$<AND:$<BOOL:${WIN32}>,$<BOOL:${USE_COMPILER_RT}>>:--rtlib=compiler-rt>
  )
  target_compile_options(${target} PRIVATE
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
    -flto
  )
  target_link_options(${target} PUBLIC
    $<$<AND:$<BOOL:${WIN32}>,$<BOOL:${USE_COMPILER_RT}>>:--rtlib=compiler-rt>
    $<$<BOOL:${WIN32}>:-no-pthread>
  )
  target_link_options(${target} PRIVATE
    -fuse-ld=lld
    -Wl,--gc-sections
    # -Wl,--print-gc-sections
    -static
  )
  target_link_libraries(${target} PRIVATE
    $<$<NOT:$<BOOL:${WIN32}>>:pthread>
    $<$<NOT:$<STREQUAL:${target},ovbase>>:ovbase>
  )
endforeach(target)
