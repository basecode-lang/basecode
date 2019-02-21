cmake_minimum_required(VERSION 3.10)
project(alpha-bootstrap)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)

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
add_subdirectory(ext/utf8proc-2.2.0 EXCLUDE_FROM_ALL)
include_directories(ext/utf8proc-2.2.0)

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
add_custom_target(basecode-configured DEPENDS dummy-target basecode bc)
add_dependencies(basecode-configured all-unit-tests)
add_custom_command(
        TARGET basecode-configured
        COMMAND ${CMAKE_COMMAND} -E echo "copy_directory: ${PROJECT_SOURCE_DIR}/tests"
        COMMAND ${CMAKE_COMMAND} -E copy_directory ${PROJECT_SOURCE_DIR}/tests ${PROJECT_BINARY_DIR}/tests
        COMMAND ${CMAKE_COMMAND} -E echo "copy_directory: ${PROJECT_SOURCE_DIR}/modules"
        COMMAND ${CMAKE_COMMAND} -E copy_directory ${PROJECT_SOURCE_DIR}/modules ${PROJECT_BINARY_DIR}/modules
)
