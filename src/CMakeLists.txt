option(USE_STR "use struct str" ON)
option(USE_WSTR "use struct wstr" ON)
option(LEAK_DETECTOR "use leak detector" ON)
option(ALLOCATE_LOGGER "use allocate logger" ON)
option(USE_ADDREDS_SANITIZER "use address sanitizer" OFF)
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
  mem.c
  mem_aligned.c
  num/char/atoi.c
  num/char/atou.c
  num/char/ftoa.c
  num/char/itoa.c
  num/char/utoa.c
  num/wchar/atoi.c
  num/wchar/atou.c
  num/wchar/ftoa.c
  num/wchar/itoa.c
  num/wchar/utoa.c
  ovbase.c
  ovthreads.c
  printf/char.c
  printf/wchar.c
  str/atoi_.c
  str/atou_.c
  str/cat_.c
  str/cat_m_.c
  str/cpy_.c
  str/cpy_m_.c
  str/itoa_.c
  str/ncat_.c
  str/ncpy_.c
  str/replace_all_.c
  str/sprintf_.c
  str/str_.c
  str/utoa_.c
  wstr/atoi_.c
  wstr/atou_.c
  wstr/cat_.c
  wstr/cat_m_.c
  wstr/cpy_.c
  wstr/cpy_m_.c
  wstr/itoa_.c
  wstr/ncat_.c
  wstr/ncpy_.c
  wstr/replace_all_.c
  wstr/sprintf_.c
  wstr/str_.c
  wstr/utoa_.c
)
list(APPEND targets ovbase)

add_executable(array_test array_test.c)
list(APPEND targets array_test)
add_executable(ovbase_test ovbase_test.c)
list(APPEND targets ovbase_test)
add_executable(hmap_test hmap/test.c)
list(APPEND targets hmap_test)
add_executable(str_test str/test.c)
list(APPEND targets str_test)
add_executable(wstr_test wstr/test.c)
list(APPEND targets wstr_test)
add_executable(num_wchar_test num/wchar/test.c)
list(APPEND targets num_wchar_test)
add_executable(num_char_test num/char/test.c)
list(APPEND targets num_char_test)
add_executable(printf_char_test printf/char_test.c)
list(APPEND targets printf_char_test)
add_executable(printf_wchar_test printf/wchar_test.c)
list(APPEND targets printf_wchar_test)

if(NOT SKIP_OVTHREADS_TEST)
  add_executable(ovthreads_test ovthreads_test.c)
  list(APPEND targets ovthreads_test)
endif()

foreach(target ${targets})
  if(target MATCHES "_test$")
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
    $<$<BOOL:${WIN32}>:_WIN32_WINNT=0x0501>
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
    -Wno-declaration-after-statement
    -Wno-padded
    -ffunction-sections
    -fdata-sections
    $<$<BOOL:${USE_ADDREDS_SANITIZER}>:-fsanitize=address>
    $<$<CONFIG:Debug>:-O0>
    $<$<CONFIG:Release>:-O2>
    -flto
  )
  target_link_options(${target} PUBLIC
    $<$<AND:$<BOOL:${WIN32}>,$<BOOL:${USE_COMPILER_RT}>>:--rtlib=compiler-rt>
    $<$<BOOL:${WIN32}>:-no-pthread>
    $<$<BOOL:${USE_ADDREDS_SANITIZER}>:-fsanitize=address>
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
    m
  )
endforeach(target)
