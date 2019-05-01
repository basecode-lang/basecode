cmake_minimum_required(VERSION 3.10)
project(alpha-bootstrap)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)

# configure ccache if available
find_program(CCACHE_FOUND ccache)
if(CCACHE_FOUND)
    set_property(GLOBAL PROPERTY RULE_LAUNCH_COMPILE ccache)
    set_property(GLOBAL PROPERTY RULE_LAUNCH_LINK ccache)
    message(STATUS "${PROJECT_NAME} using ccache.")
endif(CCACHE_FOUND)

if (UNIX AND NOT APPLE)
    set(CMAKE_CXX_FLAGS "-fPIC")
endif()

set(DL_LIBRARY "")

if ("${CMAKE_SYSTEM_NAME}" MATCHES "FreeBSD")
    add_definitions(-D__USE_ISOC99)
elseif ("${CMAKE_SYSTEM_NAME}" MATCHES "Linux")
    set(DL_LIBRARY "dl")
endif()

if ("${CMAKE_CXX_COMPILER_ID}" MATCHES "Clang")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Wno-unused-private-field -Wno-unknown-pragmas -Wno-inconsistent-missing-override" CACHE STRING "compile flags" FORCE)
    message(STATUS "${PROJECT_NAME} using clang flags: ${CMAKE_CXX_FLAGS}")
elseif ("${CMAKE_CXX_COMPILER_ID}" MATCHES "GNU")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++2a -Wall -Wno-unknown-pragmas -Wno-parentheses -Wno-sequence-point" CACHE STRING "compile flags" FORCE)
    message(STATUS "${PROJECT_NAME} using gcc flags: ${CMAKE_CXX_FLAGS}")
else ()
    message(FATAL_ERROR "${PROJECT_NAME} unknown compiler: ${CMAKE_CXX_COMPILER_ID}")
endif()

# enhance unit test discovery
enable_testing()
set_property(GLOBAL PROPERTY UNIT_TEST_TARGETS "")

macro(add_unit_test target test_name args)
    set_property(GLOBAL PROPERTY UNIT_TEST_TARGETS ${UNIT_TEST_TARGETS} ${CMAKE_CURRENT_BINARY_DIR}/${target})
    add_test(NAME ${test_name} COMMAND ${CMAKE_CURRENT_BINARY_DIR}/${target} ${args})
endmacro(add_unit_test)

# dummy target used for file copies
add_custom_target(dummy-target ALL DEPENDS custom-output)
add_custom_command(OUTPUT custom-output COMMAND ${CMAKE_COMMAND} -E echo DEPENDS always-rebuild)
add_custom_command(OUTPUT always-rebuild COMMAND ${CMAKE_COMMAND} -E echo)

# libfmt
add_subdirectory(ext/fmt-5.3.0 EXCLUDE_FROM_ALL)

# dyncall
add_subdirectory(ext/dyncall-1.0 EXCLUDE_FROM_ALL)
include_directories(ext/dyncall-1.0)

# boost
include_directories(ext/boost-1.67.0)

# boost system library
add_library(boost-system ext/boost-1.67.0/libs/system/src/error_code.cpp)

# boost filesystem library
set(
        BOOST_FILESYSTEM_SOURCES
        ext/boost-1.67.0/libs/filesystem/src/codecvt_error_category.cpp
        ext/boost-1.67.0/libs/filesystem/src/operations.cpp
        ext/boost-1.67.0/libs/filesystem/src/path.cpp
        ext/boost-1.67.0/libs/filesystem/src/path_traits.cpp
        ext/boost-1.67.0/libs/filesystem/src/portability.cpp
        ext/boost-1.67.0/libs/filesystem/src/unique_path.cpp
        ext/boost-1.67.0/libs/filesystem/src/utf8_codecvt_facet.cpp
)
if (WIN32)
    set(
        BOOST_FILESYSTEM_SOURCES
        ${BOOST_FILESYSTEM_SOURCES}
        ext/boost-1.67.0/libs/filesystem/src/windows_file_codecvt.cpp
    )
endif()
add_library(boost-filesystem ${BOOST_FILESYSTEM_SOURCES})

# utf8proc
add_subdirectory(ext/utf8proc-2.3.0 EXCLUDE_FROM_ALL)
include_directories(ext/utf8proc-2.3.0)

# ya_getopt
include_directories(ext/ya_getopt-1.0.0)

# short_alloc
include_directories(ext/short_alloc)

# ncurses
set(CURSES_NEED_NCURSES true)
find_package(Curses)
if (CURSES_FOUND)
    include_directories(${CURSES_INCLUDE_DIRS})
endif()

# gnu bfd library
include_directories(ext/bfd-2.32.0)
include_directories(ext/bfd-2.32.0/include)

add_definitions(-DBINDIR="/usr/local/bin" -DDEBUGDIR="/usr/local/lib/debug")

set(
        BFD_SOURCES
        ext/bfd-2.32.0/archive.c
        ext/bfd-2.32.0/archive64.c
        ext/bfd-2.32.0/archures.c
        ext/bfd-2.32.0/bfd.c
        ext/bfd-2.32.0/bfdio.c
        ext/bfd-2.32.0/binary.c
        ext/bfd-2.32.0/cache.c
        ext/bfd-2.32.0/coff-bfd.c
        ext/bfd-2.32.0/compress.c
        ext/bfd-2.32.0/corefile.c
        ext/bfd-2.32.0/cpu-i386.c
        ext/bfd-2.32.0/cpu-plugin.c
        ext/bfd-2.32.0/cpu-powerpc.c
        ext/bfd-2.32.0/cpu-rs6000.c
        ext/bfd-2.32.0/dwarf2.c
        ext/bfd-2.32.0/elf-properties.c
        ext/bfd-2.32.0/format.c
        ext/bfd-2.32.0/hash.c
        ext/bfd-2.32.0/ihex.c
        ext/bfd-2.32.0/init.c
        ext/bfd-2.32.0/libbfd.c
        ext/bfd-2.32.0/linker.c
        ext/bfd-2.32.0/mach-o-i386.c
        ext/bfd-2.32.0/mach-o-x86-64.c
        ext/bfd-2.32.0/mach-o.c
        ext/bfd-2.32.0/merge.c
        ext/bfd-2.32.0/opncls.c
        ext/bfd-2.32.0/pef.c
        ext/bfd-2.32.0/plugin.c
        ext/bfd-2.32.0/reloc.c
        ext/bfd-2.32.0/section.c
        ext/bfd-2.32.0/simple.c
        ext/bfd-2.32.0/srec.c
        ext/bfd-2.32.0/stab-syms.c
        ext/bfd-2.32.0/stabs.c
        ext/bfd-2.32.0/syms.c
        ext/bfd-2.32.0/targets.c
        ext/bfd-2.32.0/tekhex.c
        ext/bfd-2.32.0/verilog.c
        ext/bfd-2.32.0/xsym.c
)
if (WIN32)
    set(BFD_SOURCES ${BFD_SOURCES} ext/bfd-2.32.0/bfdwin.c)
endif ()

add_library(bfd ${BFD_SOURCES})

# projects
add_subdirectory(basecode)
add_subdirectory(bc)

# all_unit_tests target
get_property(test_targets GLOBAL PROPERTY UNIT_TEST_TARGETS)
message(STATUS "UNIT_TEST_TARGETS=${test_targets}")
add_custom_target(all-unit-tests ALL DEPENDS ${test_targets})
add_custom_command(
        TARGET all-unit-tests
        COMMENT "Execute all unit tests"
        POST_BUILD
        COMMAND ctest -C $<CONFIGURATION> --output-on-failure
        WORKING_DIRECTORY ${CMAKE_BINARY_DIR})

# custom post bulid steps
add_custom_target(basecode-configured DEPENDS dummy-target bc)
add_dependencies(basecode-configured all-unit-tests)
add_custom_command(
        TARGET basecode-configured
        COMMAND ${CMAKE_COMMAND} -E echo "copy_directory: ${PROJECT_SOURCE_DIR}/tests"
        COMMAND ${CMAKE_COMMAND} -E copy_directory ${PROJECT_SOURCE_DIR}/tests ${PROJECT_BINARY_DIR}/tests
        COMMAND ${CMAKE_COMMAND} -E echo "copy_directory: ${PROJECT_SOURCE_DIR}/modules"
        COMMAND ${CMAKE_COMMAND} -E copy_directory ${PROJECT_SOURCE_DIR}/modules ${PROJECT_BINARY_DIR}/modules
)
