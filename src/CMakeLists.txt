option(USE_STR "use struct str" ON)
option(USE_WSTR "use struct wstr" ON)
option(LEAK_DETECTOR "use leak detector" ON)
option(ALLOCATE_LOGGER "use allocate logger" ON)
option(USE_ADDRESS_SANITIZER "use address sanitizer" OFF)
option(USE_COMPILER_RT "use compiler-rt runtime" OFF)
option(USE_NO_PTHREAD "add -no-pthread" OFF)
option(USE_LTO "use LTO" ON)
option(SKIP_OVTHREADS_TEST "skip ovthreads tests" ON)
option(TARGET_EMSCRIPTEN "target emscripten" OFF)
option(TARGET_WASI_SDK "target wasi-sdk" OFF)
set(LDNAME "lld" CACHE STRING "ld name")

set(is_clang "$<C_COMPILER_ID:Clang>")
set(v16_or_later "$<VERSION_GREATER_EQUAL:$<C_COMPILER_VERSION>,16>")

set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED ON)
set(CMAKE_C_EXTENSIONS OFF)

add_library(ovbase_intf INTERFACE)
target_compile_options(ovbase_intf
INTERFACE
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
  -ffunction-sections
  -fdata-sections
  $<$<BOOL:${USE_ADDRESS_SANITIZER}>:-fsanitize=address>
  $<$<CONFIG:Debug>:-O0>
  $<$<CONFIG:Release>:-O2>
  $<$<BOOL:${USE_LTO}>:-flto>
)
target_link_options(ovbase_intf
INTERFACE
  $<$<NOT:$<STREQUAL:${LDNAME},>>:-fuse-ld=${LDNAME}>
  -Wl,--gc-sections
  # -Wl,--print-gc-sections
  -static
  $<$<BOOL:${TARGET_WASI_SDK}>:-Wl,--import-memory,--export-memory,--max-memory=67108864>
)

set(SOURCE_INCLUDE_DIR ${PROJECT_SOURCE_DIR}/include)
set(DESTINATION_INCLUDE_DIR ${PROJECT_BINARY_DIR}/include)
configure_file(${SOURCE_INCLUDE_DIR}/ovbase_config.h.in ${DESTINATION_INCLUDE_DIR}/ovbase_config.h @ONLY)
configure_file(${SOURCE_INCLUDE_DIR}/ovbase.h ${DESTINATION_INCLUDE_DIR} COPYONLY)
configure_file(${SOURCE_INCLUDE_DIR}/ovmo.h ${DESTINATION_INCLUDE_DIR} COPYONLY)
configure_file(${SOURCE_INCLUDE_DIR}/ovnum.h ${DESTINATION_INCLUDE_DIR} COPYONLY)
configure_file(${SOURCE_INCLUDE_DIR}/ovprintf.h ${DESTINATION_INCLUDE_DIR} COPYONLY)
configure_file(${SOURCE_INCLUDE_DIR}/ovtest.h ${DESTINATION_INCLUDE_DIR} COPYONLY)
configure_file(${SOURCE_INCLUDE_DIR}/ovthreads.h ${DESTINATION_INCLUDE_DIR} COPYONLY)
configure_file(${SOURCE_INCLUDE_DIR}/ovutf.h ${DESTINATION_INCLUDE_DIR} COPYONLY)
configure_file(${PROJECT_SOURCE_DIR}/3rd/acutest/include/acutest.h ${DESTINATION_INCLUDE_DIR}/ovbase_3rd/acutest.h COPYONLY)
configure_file(${PROJECT_SOURCE_DIR}/3rd/tinycthread/source/tinycthread.h ${DESTINATION_INCLUDE_DIR}/ovbase_3rd/tinycthread.h COPYONLY)

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
  hmap/iter.c
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
set_target_properties(ovbase PROPERTIES
RUNTIME_OUTPUT_DIRECTORY
  ${PROJECT_BINARY_DIR}/bin
ARCHIVE_OUTPUT_DIRECTORY
  ${PROJECT_BINARY_DIR}/lib
LIBRARY_OUTPUT_DIRECTORY
  ${PROJECT_BINARY_DIR}/lib
)
set_property(TARGET ovbase PROPERTY PUBLIC_HEADER
  ${DESTINATION_INCLUDE_DIR}/ovbase.h
  ${DESTINATION_INCLUDE_DIR}/ovbase_config.h
  ${DESTINATION_INCLUDE_DIR}/ovmo.h
  ${DESTINATION_INCLUDE_DIR}/ovnum.h
  ${DESTINATION_INCLUDE_DIR}/ovprintf.h
  ${DESTINATION_INCLUDE_DIR}/ovtest.h
  ${DESTINATION_INCLUDE_DIR}/ovthreads.h
  ${DESTINATION_INCLUDE_DIR}/ovutf.h
)
target_include_directories(ovbase
PUBLIC
  $<INSTALL_INTERFACE:../include>
  $<BUILD_INTERFACE:${PROJECT_SOURCE_DIR}/include>
  $<BUILD_INTERFACE:${PROJECT_BINARY_DIR}/include>
)
target_compile_definitions(ovbase
PRIVATE
  $<$<BOOL:${WIN32}>:_WIN32_WINNT=0x0501>
  $<$<BOOL:${UNIX}>:_POSIX_C_SOURCE=199309L>
  $<$<CONFIG:Release>:NDEBUG>
)
target_compile_options(ovbase
PUBLIC
  $<$<AND:$<BOOL:${WIN32}>,$<BOOL:${USE_COMPILER_RT}>>:--rtlib=compiler-rt>
  $<$<BOOL:${TARGET_EMSCRIPTEN}>:-sUSE_PTHREADS>
)
target_link_options(ovbase
PUBLIC
  $<$<AND:$<BOOL:${WIN32}>,$<BOOL:${USE_COMPILER_RT}>>:--rtlib=compiler-rt>
  $<$<AND:$<BOOL:${WIN32}>,$<BOOL:${USE_NO_PTHREAD}>>:-no-pthread>
  $<$<NOT:$<BOOL:${WIN32}>>:-pthread>
  $<$<BOOL:${USE_ADDRESS_SANITIZER}>:-fsanitize=address>
  $<$<BOOL:${TARGET_EMSCRIPTEN}>:-sUSE_PTHREADS -sSTANDALONE_WASM>
PRIVATE
  -static
)
target_link_libraries(ovbase
PRIVATE
  m
  ovbase_intf
)
install(TARGETS ovbase
        EXPORT libovbase
        RUNTIME DESTINATION bin
        ARCHIVE DESTINATION lib
        LIBRARY DESTINATION lib
        PUBLIC_HEADER DESTINATION include)
install(
FILES
  ${DESTINATION_INCLUDE_DIR}/ovbase_3rd/tinycthread.h
  ${DESTINATION_INCLUDE_DIR}/ovbase_3rd/acutest.h
DESTINATION
  include/ovbase_3rd
)
add_executable(array_test array_test.c)
list(APPEND tests array_test)
add_executable(ovbase_test ovbase_test.c)
list(APPEND tests ovbase_test)
add_executable(hmap_test hmap/test.c)
list(APPEND tests hmap_test)
add_executable(str_test str/test.c)
list(APPEND tests str_test)
add_executable(wstr_test wstr/test.c)
list(APPEND tests wstr_test)
add_executable(mo_test mo/test.c $<$<BOOL:${WIN32}>:mo/test_win32/test.rc>)
list(APPEND tests mo_test)
add_executable(num_wchar_test num/wchar/test.c)
list(APPEND tests num_wchar_test)
add_executable(num_char_test num/char/test.c)
list(APPEND tests num_char_test)
add_executable(printf_char_test printf/char_test.c)
list(APPEND tests printf_char_test)
add_executable(printf_wchar_test printf/wchar_test.c)
list(APPEND tests printf_wchar_test)
add_executable(utf_test utf/test.c)
list(APPEND tests utf_test)

if(NOT SKIP_OVTHREADS_TEST)
  add_executable(ovthreads_test ovthreads_test.c)
  list(APPEND tests ovthreads_test)
endif()

foreach(target ${tests})
  if(TARGET_EMSCRIPTEN OR TARGET_WASI_SDK)
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
  target_compile_definitions(${target}
  PRIVATE
    SOURCE_DIR="${CMAKE_CURRENT_SOURCE_DIR}"
    SOURCE_DIR_WIDE=L"${CMAKE_CURRENT_SOURCE_DIR}"
  )
  target_link_libraries(${target}
  PRIVATE
    m
    ovbase
    ovbase_intf
  )
endforeach(target)
