cmake_minimum_required(VERSION 3.0)
project(ovbase C)

file(GLOB sources "${PROJECT_SOURCE_DIR}/src/*.c" "${PROJECT_SOURCE_DIR}/include/*.h")
find_program(CLANG_FORMAT_EXE clang-format)
add_custom_target(${PROJECT_NAME}-format ALL
  COMMAND ${CLANG_FORMAT_EXE} -style=file -i ${sources}
)

enable_testing()
add_subdirectory(src)
