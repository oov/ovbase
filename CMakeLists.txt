cmake_minimum_required(VERSION 3.0)
project(ovbase C)
enable_testing()

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

add_subdirectory(src)
