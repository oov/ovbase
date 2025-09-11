option(LEAK_DETECTOR "use leak detector" ON)
option(ALLOCATE_LOGGER "use allocate logger" ON)
option(USE_ADDRESS_SANITIZER "use address sanitizer" OFF)
option(USE_COMPILER_RT "use compiler-rt runtime" OFF)
option(USE_NO_PTHREAD "add -no-pthread" OFF)
option(USE_LTO "use LTO" OFF)
option(SKIP_OVTHREADS_TEST "skip ovthreads tests" ON)
option(TARGET_EMSCRIPTEN "target emscripten" OFF)
option(TARGET_WASI_SDK "target wasi-sdk" OFF)
set(LDNAME "lld" CACHE STRING "ld name")

set(is_clang "$<C_COMPILER_ID:Clang>")
set(v16_or_later "$<VERSION_GREATER_EQUAL:$<C_COMPILER_VERSION>,16>")
set(v18_or_later "$<VERSION_GREATER_EQUAL:$<C_COMPILER_VERSION>,18>")
set(v19_or_later "$<VERSION_GREATER_EQUAL:$<C_COMPILER_VERSION>,19>")
set(v21_or_later "$<VERSION_GREATER_EQUAL:$<C_COMPILER_VERSION>,21>")

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
  $<$<AND:${is_clang},${v21_or_later}>:-Wno-c++-keyword>
  $<$<AND:${is_clang},${v19_or_later}>:-Wno-pre-c11-compat>
  $<$<AND:${is_clang},${v18_or_later}>:-Wno-switch-default>
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
file(MAKE_DIRECTORY ${DESTINATION_INCLUDE_DIR})
configure_file(${SOURCE_INCLUDE_DIR}/ovarray.h ${DESTINATION_INCLUDE_DIR} COPYONLY)
configure_file(${SOURCE_INCLUDE_DIR}/ovbase_config.h.in ${DESTINATION_INCLUDE_DIR}/ovbase_config.h @ONLY)
configure_file(${SOURCE_INCLUDE_DIR}/ovbase.h ${DESTINATION_INCLUDE_DIR} COPYONLY)
configure_file(${SOURCE_INCLUDE_DIR}/ovcyrb64.h ${DESTINATION_INCLUDE_DIR} COPYONLY)
configure_file(${SOURCE_INCLUDE_DIR}/ovrand.h ${DESTINATION_INCLUDE_DIR} COPYONLY)
configure_file(${SOURCE_INCLUDE_DIR}/ovhashmap.h ${DESTINATION_INCLUDE_DIR} COPYONLY)
configure_file(${SOURCE_INCLUDE_DIR}/ovmo.h ${DESTINATION_INCLUDE_DIR} COPYONLY)
configure_file(${SOURCE_INCLUDE_DIR}/ovnum.h ${DESTINATION_INCLUDE_DIR} COPYONLY)
configure_file(${SOURCE_INCLUDE_DIR}/ovprintf.h ${DESTINATION_INCLUDE_DIR} COPYONLY)
configure_file(${SOURCE_INCLUDE_DIR}/ovprintf_ex.h ${DESTINATION_INCLUDE_DIR} COPYONLY)
configure_file(${SOURCE_INCLUDE_DIR}/ovtest.h ${DESTINATION_INCLUDE_DIR} COPYONLY)
configure_file(${SOURCE_INCLUDE_DIR}/ovthreads.h ${DESTINATION_INCLUDE_DIR} COPYONLY)
configure_file(${SOURCE_INCLUDE_DIR}/ovutf.h ${DESTINATION_INCLUDE_DIR} COPYONLY)
configure_file(${PROJECT_SOURCE_DIR}/3rd/acutest/include/acutest.h ${DESTINATION_INCLUDE_DIR}/ovbase_3rd/acutest.h COPYONLY)
configure_file(${PROJECT_SOURCE_DIR}/3rd/tinycthread/source/tinycthread.h ${DESTINATION_INCLUDE_DIR}/ovbase_3rd/tinycthread.h COPYONLY)

# Prepare source file lists
set(OVBASE_SOURCES
  array.c
  error.c
  error_report.c
  hashmap/common.c
  hashmap/clear.c
  hashmap/count.c
  hashmap/delete.c
  hashmap/destroy.c
  hashmap/get.c
  hashmap/iter.c
  hashmap/create_dynamic.c
  hashmap/create_static.c
  hashmap/set.c
  mem.c
  mem_aligned.c
  mo/mo.c
  num/char/atoi.c
  num/char/atof.c
  num/char/atou.c
  num/char/ftoa.c
  num/char/itoa.c
  num/char/utoa.c
  num/char16/atoi.c
  num/char16/atof.c
  num/char16/atou.c
  num/char16/ftoa.c
  num/char16/itoa.c
  num/char16/utoa.c
  num/char32/atoi.c
  num/char32/atof.c
  num/char32/atou.c
  num/char32/ftoa.c
  num/char32/itoa.c
  num/char32/utoa.c
  num/wchar/atoi.c
  num/wchar/atof.c
  num/wchar/atou.c
  num/wchar/ftoa.c
  num/wchar/itoa.c
  num/wchar/utoa.c
  output.c
  ovbase.c
  ovthreads.c
  printf/char.c
  printf/wchar.c
  printf_ex/char.c
  printf_ex/wchar.c
  utf/codepoint_char16.c
  utf/codepoint_char32.c
  utf/codepoint_sjis.c
  utf/codepoint_utf8.c
  utf/codepoint_wchar.c
  utf/common.c
  utf/to_char16.c
  utf/to_char32.c
  utf/to_wchar.c
  utf/utf8_from_char16.c
  utf/utf8_from_char32.c
  utf/utf8_from_sjis.c
  utf/utf8_from_wchar.c
)

if(WIN32)
  list(APPEND OVBASE_SOURCES mo/win32.c)
endif()

add_library(ovbase STATIC ${OVBASE_SOURCES})
set_target_properties(ovbase PROPERTIES
RUNTIME_OUTPUT_DIRECTORY
  ${PROJECT_BINARY_DIR}/bin
ARCHIVE_OUTPUT_DIRECTORY
  ${PROJECT_BINARY_DIR}/lib
LIBRARY_OUTPUT_DIRECTORY
  ${PROJECT_BINARY_DIR}/lib
)
set_property(TARGET ovbase PROPERTY PUBLIC_HEADER
  ${DESTINATION_INCLUDE_DIR}/ovarray.h
  ${DESTINATION_INCLUDE_DIR}/ovbase.h
  ${DESTINATION_INCLUDE_DIR}/ovbase_config.h
  ${DESTINATION_INCLUDE_DIR}/ovcyrb64.h
  ${DESTINATION_INCLUDE_DIR}/ovrand.h
  ${DESTINATION_INCLUDE_DIR}/ovhashmap.h
  ${DESTINATION_INCLUDE_DIR}/ovmo.h
  ${DESTINATION_INCLUDE_DIR}/ovnum.h
  ${DESTINATION_INCLUDE_DIR}/ovprintf.h
  ${DESTINATION_INCLUDE_DIR}/ovtest.h
  ${DESTINATION_INCLUDE_DIR}/ovthreads.h
  ${DESTINATION_INCLUDE_DIR}/ovutf.h
)
target_include_directories(ovbase
PUBLIC
  $<INSTALL_INTERFACE:include>
  $<BUILD_INTERFACE:${PROJECT_BINARY_DIR}/include>
PRIVATE
  $<$<BOOL:${USE_MIMALLOC}>:${LOCAL_INSTALL_PREFIX}/include/mimalloc-2.1>
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
  $<$<BOOL:${USE_MIMALLOC}>:mimalloc-static>
)
install(TARGETS ovbase ovbase_intf
        EXPORT ovbase-export
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
install(EXPORT ovbase-export
        FILE ovbase-config.cmake
        DESTINATION share/cmake/ovbase/
        EXPORT_LINK_INTERFACE_LIBRARIES)
export(EXPORT ovbase-export
       FILE "${CMAKE_BINARY_DIR}/ovbase-config.cmake")

add_executable(array_test array_test.c)
list(APPEND tests array_test)
add_executable(error_test error_test.c)
list(APPEND tests error_test)
add_executable(hashmap_test hashmap/test.c)
list(APPEND tests hashmap_test)
add_executable(mo_test mo/test.c $<$<BOOL:${WIN32}>:mo/test_win32/test.rc>)
list(APPEND tests mo_test)
add_executable(num_wchar_test num/wchar/test.c)
list(APPEND tests num_wchar_test)
add_executable(num_char_test num/char/test.c)
list(APPEND tests num_char_test)
add_executable(num_char16_test num/char16/test.c)
list(APPEND tests num_char16_test)
add_executable(num_char32_test num/char32/test.c)
list(APPEND tests num_char32_test)
add_executable(printf_char_test printf/char_test.c)
list(APPEND tests printf_char_test)
add_executable(printf_wchar_test printf/wchar_test.c)
list(APPEND tests printf_wchar_test)
add_executable(printf_ex_test printf_ex/test.c)
list(APPEND tests printf_ex_test)
add_executable(utf_test utf/test.c)
list(APPEND tests utf_test)
add_executable(mem_test mem_test.c)
list(APPEND tests mem_test)
add_executable(mem_aligned_test mem_aligned_test.c)
list(APPEND tests mem_aligned_test)

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
          --wasi threads
          $<TARGET_FILE:${target}>
    )
  else()
    add_test(NAME ${target} COMMAND $<TARGET_FILE:${target}>)
  endif()
  target_compile_definitions(${target}
  PRIVATE
    OVBASE_SOURCE_DIR="${CMAKE_CURRENT_SOURCE_DIR}"
    OVBASE_SOURCE_DIR_WIDE=L"${CMAKE_CURRENT_SOURCE_DIR}"
  )
  target_link_libraries(${target}
  PRIVATE
    m
    ovbase
    ovbase_intf
  )
endforeach(target)
