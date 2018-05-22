cmake_minimum_required(VERSION 3.11)
project(bootstrap)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

if ("${CMAKE_CXX_COMPILER_ID}" MATCHES "Clang")
    set(CMAKE_CXX_FLAGS "-Wall -Wno-unused-private-field -Wno-unknown-pragmas -Wno-inconsistent-missing-override" CACHE STRING "compile flags" FORCE)
    message(STATUS "Using clang flags: ${CMAKE_CXX_FLAGS}")
elseif ("${CMAKE_CXX_COMPILER_ID}" MATCHES "GNU")
    set(CMAKE_CXX_FLAGS "-Wall" CACHE STRING "compile flags" FORCE)
    message(STATUS "Using gcc flags: ${CMAKE_CXX_FLAGS}")
else ()
    message(STATUS "Unknown compiler: ${CMAKE_CXX_COMPILER_ID}")
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

# conan configuration
set(CONAN_DISABLE_CHECK_COMPILER true)
include(conanbuildinfo.cmake)
conan_basic_setup()

include_directories(
    #${PROJECT_BINARY_DIR}
    ${PROJECT_SOURCE_DIR}
)

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

# actual executable target
add_executable(
    bootstrap
    bytes.h
    main.cpp
    result.h
    ast.cpp ast.h
    terp.cpp terp.h
    result_message.h
    lexer.cpp lexer.h
    token.cpp token.h
    scope.cpp scope.h
    parser.cpp parser.h
    compiler.cpp compiler.h
    symbol_table.cpp symbol_table.h
    hex_formatter.cpp hex_formatter.h
    instruction_emitter.cpp instruction_emitter.h)
target_link_libraries(bootstrap ${CONAN_LIBS})

# custom post bulid steps
add_custom_target(bootstrap-configured DEPENDS dummy-target bootstrap)
add_dependencies(bootstrap-configured all-unit-tests)
add_custom_command(
    TARGET bootstrap-configured
    COMMAND ${CMAKE_COMMAND} -E echo "copy_directory: ${PROJECT_SOURCE_DIR}/examples"
    COMMAND ${CMAKE_COMMAND} -E copy_directory ${PROJECT_SOURCE_DIR}/examples ${PROJECT_BINARY_DIR}/examples
)
