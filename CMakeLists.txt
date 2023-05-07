cmake_minimum_required(VERSION 3.0)

set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED ON)
set(CMAKE_C_EXTENSIONS OFF)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

option(FORMAT_SOURCES "execute clang-format" ON)

if(FORMAT_SOURCES)
  file(GLOB sources
    "${PROJECT_SOURCE_DIR}/include/*.h"
    "${PROJECT_SOURCE_DIR}/src/*.c"
    "${PROJECT_SOURCE_DIR}/src/*.h"
    "${PROJECT_SOURCE_DIR}/src/*/*.c"
    "${PROJECT_SOURCE_DIR}/src/*/*.h"
    "${PROJECT_SOURCE_DIR}/src/*/*/*.c"
    "${PROJECT_SOURCE_DIR}/src/*/*/*.h"
  )
  find_program(CLANG_FORMAT_EXE clang-format)
  add_custom_target(${PROJECT_NAME}-format ALL
    COMMAND ${CLANG_FORMAT_EXE} -style=file -i ${sources}
  )
endif()

project(ovbase C)
if(WIN32)
  enable_language(RC)
endif()
enable_testing()

add_subdirectory(src)
