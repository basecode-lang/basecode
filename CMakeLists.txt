cmake_minimum_required(VERSION 3.11)
project(alpha-bootstrap)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# conan configuration
set(CONAN_DISABLE_CHECK_COMPILER true)
include(conanbuildinfo.cmake)
conan_basic_setup(KEEP_RPATHS)

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

# projects
add_subdirectory(alpha-core)
add_subdirectory(bac)

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
add_custom_target(alpha-bootstrap-configured DEPENDS dummy-target alpha-core bac)
add_dependencies(alpha-bootstrap-configured all-unit-tests)
add_custom_command(
        TARGET alpha-bootstrap-configured
        COMMAND ${CMAKE_COMMAND} -E echo "copy_directory: ${PROJECT_SOURCE_DIR}/examples"
        COMMAND ${CMAKE_COMMAND} -E copy_directory ${PROJECT_SOURCE_DIR}/examples ${PROJECT_BINARY_DIR}/examples
)
