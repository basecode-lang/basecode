cmake_minimum_required(VERSION 3.11)
project(bootstrap)
include_directories(${PROJECT_SOURCE_DIR})

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

        main.cpp

        common/bytes.h
        common/result.h
        common/result_message.h
        common/hex_formatter.cpp common/hex_formatter.h

        parser/ast.cpp parser/ast.h
        parser/lexer.cpp parser/lexer.h
        parser/token.cpp parser/token.h
        parser/parser.cpp parser/parser.h

        vm/terp.cpp vm/terp.h
        vm/instruction_emitter.cpp vm/instruction_emitter.h

        compiler/scope.cpp compiler/scope.h
        compiler/bytecode_emitter.cpp compiler/bytecode_emitter.h
        compiler/symbol_table.cpp compiler/symbol_table.h)
target_link_libraries(bootstrap ${CONAN_LIBS})

# custom post bulid steps
add_custom_target(bootstrap-configured DEPENDS dummy-target bootstrap)
add_dependencies(bootstrap-configured all-unit-tests)
add_custom_command(
    TARGET bootstrap-configured
    COMMAND ${CMAKE_COMMAND} -E echo "copy_directory: ${PROJECT_SOURCE_DIR}/examples"
    COMMAND ${CMAKE_COMMAND} -E copy_directory ${PROJECT_SOURCE_DIR}/examples ${PROJECT_BINARY_DIR}/examples
)
