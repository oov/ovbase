cmake_minimum_required(VERSION 3.20)

option(FORMAT_SOURCES "execute clang-format" ON)
option(USE_MIMALLOC "use mimalloc" OFF)

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

if (USE_MIMALLOC)
  set(LOCAL_INSTALL_PREFIX ${PROJECT_BINARY_DIR}/local)
  include(ExternalProject)
  if (CMAKE_BUILD_TYPE STREQUAL "Debug")
    set(mimalloc_libname "libmimalloc-static-debug.a")
  else()
    set(mimalloc_libname "libmimalloc-static.a")
  endif()
  ExternalProject_Add(mimalloc
    PREFIX "${LOCAL_INSTALL_PREFIX}/3rd/mimalloc"
    SOURCE_DIR "${PROJECT_SOURCE_DIR}/3rd/mimalloc"
    CMAKE_ARGS "-DCMAKE_INSTALL_PREFIX=${LOCAL_INSTALL_PREFIX}" "-DCMAKE_BUILD_TYPE=${CMAKE_BUILD_TYPE}" -DMI_OVERRIDE=OFF -DMI_BUILD_SHARED=OFF -DMI_BUILD_OBJECT=OFF -DMI_BUILD_TESTS=OFF
    BINARY_DIR "${LOCAL_INSTALL_PREFIX}/build/mimalloc"
    BUILD_BYPRODUCTS "${LOCAL_INSTALL_PREFIX}/lib/mimalloc-2.1/${mimalloc_libname}"
  )

  add_library(mimalloc-static STATIC IMPORTED)
  set_target_properties(mimalloc-static PROPERTIES IMPORTED_LOCATION ${LOCAL_INSTALL_PREFIX}/lib/mimalloc-2.1/${mimalloc_libname})
  add_dependencies(mimalloc-static mimalloc)
endif()

add_subdirectory(src)
