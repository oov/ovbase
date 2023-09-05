cmake_minimum_required(VERSION 3.20)

option(FORMAT_SOURCES "execute clang-format" ON)

project(ovbase
        HOMEPAGE_URL "https://github.com/oov/ovbase"
        LANGUAGES C
)
if(WIN32)
  enable_language(RC)
endif()
enable_testing()

if(FORMAT_SOURCES)
  file(GLOB_RECURSE sources LIST_DIRECTORIES false CONFIGURE_DEPENDS "include/*.h" "src/*.h" "src/*.c")
  find_program(CLANG_FORMAT_EXE clang-format)
  add_custom_target(${PROJECT_NAME}-format ALL
    COMMAND ${CLANG_FORMAT_EXE} -style=file -i ${sources}
  )
endif()

add_subdirectory(src)
