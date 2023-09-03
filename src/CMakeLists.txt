option(USE_STR "use struct str" ON)
option(USE_WSTR "use struct wstr" ON)
option(LEAK_DETECTOR "use leak detector" ON)
option(ALLOCATE_LOGGER "use allocate logger" ON)
option(USE_ADDRESS_SANITIZER "use address sanitizer" OFF)
option(USE_COMPILER_RT "use compiler-rt runtime" OFF)
option(USE_LTO "use LTO" ON)
option(SKIP_OVTHREADS_TEST "skip ovthreads tests" ON)
option(TARGET_WASI_SDK "target wasi-sdk" OFF)
set(LDNAME "lld" CACHE STRING "ld name")

add_library(ovbase STATIC
  array.c
  error.c
  error_i18n.c
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
  mo/mo.c
  mo/str.c
  mo/wstr.c
  $<$<BOOL:${WIN32}>:mo/win32.c>
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
  str/to_str_.c
  str/utoa_.c
  utf/common.c
  utf/sjis.c
  utf/u2w.c
  utf/w2u.c
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
  wstr/to_wstr_.c
  wstr/utoa_.c
)
set_property(TARGET ovbase PROPERTY PUBLIC_HEADER
  ../include/ovbase.h
  ../include/ovmo.h
  ../include/ovnum.h
  ../include/ovprintf.h
  ../include/ovtest.h
  ../include/ovthreads.h
  ../include/ovutf.h
)
target_include_directories(ovbase INTERFACE
  $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>
  $<INSTALL_INTERFACE:../include>
)
install(TARGETS ovbase
        EXPORT libovbase
        ARCHIVE DESTINATION lib
        LIBRARY DESTINATION lib
        PUBLIC_HEADER DESTINATION include)
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
add_executable(mo_test mo/test.c $<$<BOOL:${WIN32}>:mo/test_win32/test.rc>)
list(APPEND targets mo_test)
add_executable(num_wchar_test num/wchar/test.c)
list(APPEND targets num_wchar_test)
add_executable(num_char_test num/char/test.c)
list(APPEND targets num_char_test)
add_executable(printf_char_test printf/char_test.c)
list(APPEND targets printf_char_test)
add_executable(printf_wchar_test printf/wchar_test.c)
list(APPEND targets printf_wchar_test)
add_executable(utf_test utf/test.c)
list(APPEND targets utf_test)

if(NOT SKIP_OVTHREADS_TEST)
  add_executable(ovthreads_test ovthreads_test.c)
  list(APPEND targets ovthreads_test)
endif()

set(is_clang "$<C_COMPILER_ID:Clang>")
set(v16_or_later "$<VERSION_GREATER_EQUAL:$<C_COMPILER_VERSION>,16>")
set(v17_or_later "$<VERSION_GREATER_EQUAL:$<C_COMPILER_VERSION>,17>")
foreach(target ${targets})
  if(target MATCHES "_test$")
    if(TARGET_WASI_SDK)
      add_test(
        NAME
          ${target}
        COMMAND
          wasmtime
            --dir=${CMAKE_CURRENT_SOURCE_DIR}/mo/test
            --wasm-features threads
            --wasi-modules experimental-wasi-threads
            ${target} -- --exec=never
      )
    else()
      add_test(NAME ${target} COMMAND ${target})
    endif()
    target_compile_definitions(${target} PRIVATE
      $<$<BOOL:${TARGET_WASI_SDK}>:ACUTEST_NOSETJMP>
      SOURCE_DIR="${CMAKE_CURRENT_SOURCE_DIR}"
      SOURCE_DIR_WIDE=L"${CMAKE_CURRENT_SOURCE_DIR}"
    )
  endif()
  target_include_directories(${target} PUBLIC
    "${PROJECT_SOURCE_DIR}/include"
  )
  target_compile_definitions(${target} PUBLIC
    $<$<BOOL:${WIN32}>:__STDC_NO_THREADS__>
    $<$<BOOL:${TARGET_WASI_SDK}>:OVBASE_DISABLE_PTHREAD_EXIT __STDC_NO_THREADS__ _POSIX_C_SOURCE=199309L _WASI_EMULATED_SIGNAL _TTHREAD_MTX_T_SIZE_=6 _TTHREAD_CND_T_SIZE_=12>
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
    $<$<AND:${is_clang},${v16_or_later}>:-Wno-unsafe-buffer-usage>
    $<$<AND:${is_clang},${v17_or_later}>:-Wno-c2x-extensions>
    -ffunction-sections
    -fdata-sections
    $<$<BOOL:${USE_ADDRESS_SANITIZER}>:-fsanitize=address>
    $<$<CONFIG:Debug>:-O0>
    $<$<CONFIG:Release>:-O2>
    $<$<BOOL:${USE_LTO}>:-flto>
  )
  target_link_options(${target} PUBLIC
    $<$<AND:$<BOOL:${WIN32}>,$<BOOL:${USE_COMPILER_RT}>>:--rtlib=compiler-rt>
    $<$<BOOL:${WIN32}>:-no-pthread>
    $<$<NOT:$<BOOL:${WIN32}>>:-pthread>
    $<$<BOOL:${USE_ADDRESS_SANITIZER}>:-fsanitize=address>
  )
  target_link_options(${target} PRIVATE
    $<$<NOT:$<STREQUAL:${LDNAME},>>:-fuse-ld=${LDNAME}>
    -Wl,--gc-sections
    # -Wl,--print-gc-sections
    -static
    $<$<BOOL:${TARGET_WASI_SDK}>:-Wl,--import-memory,--export-memory,--max-memory=67108864>
  )
  target_link_libraries(${target} PRIVATE
    $<$<NOT:$<STREQUAL:${target},ovbase>>:ovbase>
    m
  )
endforeach(target)
