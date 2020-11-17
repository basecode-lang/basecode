cmake_minimum_required(VERSION 3.15)
project(basecode-vm)

set(CMAKE_CXX_STANDARD 20)
enable_language(C CXX ASM)

set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

find_program(CCACHE_FOUND ccache)
if(CCACHE_FOUND)
    set_property(GLOBAL PROPERTY RULE_LAUNCH_COMPILE ccache)
    set_property(GLOBAL PROPERTY RULE_LAUNCH_LINK ccache)
    message(STATUS "${PROJECT_NAME} using ccache.")
endif(CCACHE_FOUND)

set(CMAKE_ASM_FLAGS "${CMAKE_ASM_FLAGS} -x assembler-with-cpp")

if (UNIX AND NOT APPLE)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fPIC")
endif()

set(DL_LIBRARY "")

if ("${CMAKE_SYSTEM_NAME}" MATCHES "FreeBSD")
    add_definitions(-D__USE_ISOC99)
elseif ("${CMAKE_SYSTEM_NAME}" MATCHES "Linux")
    set(DL_LIBRARY "dl")
endif()

if ("${CMAKE_CXX_COMPILER_ID}" MATCHES "Clang")
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wno-string-plus-int" CACHE STRING "compile flags" FORCE)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Wno-cast-qual -Wno-string-plus-int -Wno-auto-var-id -Wno-unused-private-field -Wno-unknown-pragmas -Wno-inconsistent-missing-override -Wno-c99-designator" CACHE STRING "compile flags" FORCE)
    message(STATUS "${PROJECT_NAME} using clang flags: ${CMAKE_CXX_FLAGS}")
elseif ("${CMAKE_CXX_COMPILER_ID}" MATCHES "GNU")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++2a -Wall -Wno-string-plus-int -Wno-unknown-pragmas -Wno-parentheses -Wno-sequence-point -Wno-c99-designator" CACHE STRING "compile flags" FORCE)
    message(STATUS "${PROJECT_NAME} using gcc flags: ${CMAKE_CXX_FLAGS}")
else ()
    message(FATAL_ERROR "${PROJECT_NAME} unknown compiler: ${CMAKE_CXX_COMPILER_ID}")
endif()

if(${CMAKE_SYSTEM_NAME} MATCHES "Darwin")
    set(CMAKE_C_ARCHIVE_CREATE   "<CMAKE_AR> Scr <TARGET> <LINK_FLAGS> <OBJECTS>")
    set(CMAKE_CXX_ARCHIVE_CREATE "<CMAKE_AR> Scr <TARGET> <LINK_FLAGS> <OBJECTS>")
    set(CMAKE_C_ARCHIVE_FINISH   "<CMAKE_RANLIB> -no_warning_for_no_symbols -c <TARGET>")
    set(CMAKE_CXX_ARCHIVE_FINISH "<CMAKE_RANLIB> -no_warning_for_no_symbols -c <TARGET>")
endif()

option (FORCE_COLORED_OUTPUT "Always produce ANSI-colored output (GNU/Clang only)." TRUE)
if (${FORCE_COLORED_OUTPUT})
    if ("${CMAKE_CXX_COMPILER_ID}" STREQUAL "GNU")
        add_compile_options (-fdiagnostics-color=always)
    elseif ("${CMAKE_CXX_COMPILER_ID}" STREQUAL "Clang")
        add_compile_options (-fcolor-diagnostics)
    endif ()
endif ()

# -----------------------------------------------------------------------------
#
# Dependencies
#
# -----------------------------------------------------------------------------

# elfio
include_directories(ext/elfio-master)

# asmjit
include_directories(ext/asmjit-master)
set(ASMJIT_STATIC 1)
add_subdirectory(ext/asmjit-master EXCLUDE_FROM_ALL)

# bzip2
include_directories(ext/bzip2-1.0.8)
set(
        BZIP2_SOURCES
        ext/bzip2-1.0.8/blocksort.c
        ext/bzip2-1.0.8/huffman.c
        ext/bzip2-1.0.8/crctable.c
        ext/bzip2-1.0.8/randtable.c
        ext/bzip2-1.0.8/compress.c
        ext/bzip2-1.0.8/decompress.c
        ext/bzip2-1.0.8/bzlib.c
)
add_definitions(-D_FILE_OFFSET_BITS=64)
add_library(bz2 STATIC ${BZIP2_SOURCES})
set(BZIP2_INCLUDE_DIRS "${CMAKE_SOURCE_DIR}/ext/bzip2-1.0.8")
set(BZIP2_LIBRARIES "${CMAKE_LIBRARY_OUTPUT_DIRECTORY}/libbz2.a")

# zlib
include_directories(ext/zlib-1.2.11)
add_subdirectory(ext/zlib-1.2.11 EXCLUDE_FROM_ALL)
set(ZLIB_FOUND ON)
set(ZLIB_INCLUDE_DIRS "${CMAKE_SOURCE_DIR}/ext/zlib-1.2.11")
set(ZLIB_LIBRARY "${CMAKE_LIBRARY_OUTPUT_DIRECTORY}/libz.a")
set(ZLIB_LIBRARIES "${ZLIB_LIBRARY}")

# tracy
include_directories(ext/tracy-master)

# dyncall
include_directories(ext/dyncall-1.1.0)
add_subdirectory(ext/dyncall-1.1.0 EXCLUDE_FROM_ALL)

# libpng
option(PNG_STATIC "" ON)
option(PNG_SHARED "" OFF)
option(PNG_TESTS  "" OFF)
include_directories(ext/libpng-1.6.37)
add_subdirectory(ext/libpng-1.6.37 EXCLUDE_FROM_ALL)
set(PNG_PNG_INCLUDE_DIRS "${CMAKE_SOURCE_DIR}/ext/libpng-1.6.37")
set(PNG_LIBRARY "${CMAKE_LIBRARY_OUTPUT_DIRECTORY}/libpng16d.a")

# harfbuzz
#
# N.B. harfbuzz and freetype have a circular dependency.  these
# variables need to be set at this point so harfbuzz's find_package(freetype)
# invocation succeeds and points at our internal version of freetype.
#
set(FREETYPE_INCLUDE_DIRS "${CMAKE_SOURCE_DIR}/ext/freetype-2.9.1/include")
set(FREETYPE_LIBRARY "${CMAKE_LIBRARY_OUTPUT_DIRECTORY}/libfreetyped.a")

include_directories(ext/harfbuzz-2.6.2/src)
option(HB_HAVE_FREETYPE "" ON)
add_subdirectory(ext/harfbuzz-2.6.2 EXCLUDE_FROM_ALL)
set(HARFBUZZ_INCLUDE_DIRS "${CMAKE_SOURCE_DIR}/ext/harfbuzz-2.6.2/src")
set(HARFBUZZ_LIBRARIES "${CMAKE_LIBRARY_OUTPUT_DIRECTORY}/libharfbuzz.a")

# freetype
include_directories(ext/freetype-2.9.1/include)
add_subdirectory(ext/freetype-2.9.1 EXCLUDE_FROM_ALL)

# glfw
option(GLFW_BUILD_EXAMPLES "" OFF)
option(GLFW_BUILD_TESTS "" OFF)
option(GLFW_BUILD_DOCS "" OFF)
option(GLFW_INSTALL "" OFF)
add_subdirectory(ext/glfw-3.3 EXCLUDE_FROM_ALL)
include_directories(ext/glfw-3.3/include)

# foundation
include_directories(ext/foundation/include)
add_subdirectory(ext/foundation EXCLUDE_FROM_ALL)

# -----------------------------------------------------------------------------
#
# Project
#
# -----------------------------------------------------------------------------

include_directories(
        ${CMAKE_SOURCE_DIR}/include
        ${CMAKE_BINARY_DIR}/include
)

add_subdirectory(src/libvm)
add_subdirectory(src/vmgen)
add_subdirectory(src/workbench)
