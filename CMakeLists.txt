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

set(LOCAL_INSTALL_PREFIX ${PROJECT_BINARY_DIR}/local)
include(ExternalProject)
ExternalProject_Add(mimalloc
  PREFIX "${LOCAL_INSTALL_PREFIX}/3rd/mimalloc"
  SOURCE_DIR "${PROJECT_SOURCE_DIR}/3rd/mimalloc"
  CMAKE_ARGS "-DCMAKE_INSTALL_PREFIX=${LOCAL_INSTALL_PREFIX}" "-DCMAKE_BUILD_TYPE=${CMAKE_BUILD_TYPE}" -DMI_OVERRIDE=OFF -DMI_BUILD_SHARED=OFF -DMI_BUILD_OBJECT=OFF -DMI_BUILD_TESTS=OFF
  BINARY_DIR "${LOCAL_INSTALL_PREFIX}/build/mimalloc"
)

add_subdirectory(src)
