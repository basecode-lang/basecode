cmake_minimum_required(VERSION 3.10)
project(basecode)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# configure ccache if available
find_program(CCACHE_FOUND ccache)
if(CCACHE_FOUND)
    set_property(GLOBAL PROPERTY RULE_LAUNCH_COMPILE ccache)
    set_property(GLOBAL PROPERTY RULE_LAUNCH_LINK ccache)
    message(STATUS "${PROJECT_NAME} using ccache.")
endif(CCACHE_FOUND)

set(DL_LIBRARY "")

if (UNIX AND NOT APPLE)
    set(CMAKE_CXX_FLAGS "-fPIC")
endif()

if ("${CMAKE_SYSTEM_NAME}" MATCHES "FreeBSD")
    add_definitions(-D__USE_ISOC99)
elseif ("${CMAKE_SYSTEM_NAME}" MATCHES "Linux")
    set(DL_LIBRARY "dl")
endif()

if ("${CMAKE_CXX_COMPILER_ID}" MATCHES "Clang")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Wno-unused-private-field -Wno-unknown-pragmas -Wno-inconsistent-missing-override" CACHE STRING "compile flags" FORCE)
    message(STATUS "${PROJECT_NAME} using clang flags: ${CMAKE_CXX_FLAGS}")
elseif ("${CMAKE_CXX_COMPILER_ID}" MATCHES "GNU")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++2a -Wall -Wno-unknown-pragmas -Wno-parentheses" CACHE STRING "compile flags" FORCE)
    message(STATUS "${PROJECT_NAME} using gcc flags: ${CMAKE_CXX_FLAGS}")
else ()
    message(FATAL_ERROR "${PROJECT_NAME} unknown compiler: ${CMAKE_CXX_COMPILER_ID}")
endif()

if (WIN32)
    set(SHARED_LIBRARY_PREFIX "")
    set(SHARED_LIBRARY_SUFFIX ".dll")
elseif (APPLE)
    set(SHARED_LIBRARY_PREFIX "lib")
    set(SHARED_LIBRARY_SUFFIX ".dylib")
else()
    set(SHARED_LIBRARY_PREFIX "lib")
    set(SHARED_LIBRARY_SUFFIX ".so")
endif()

set (VER_MAJOR 0)
set (VER_MINOR 1)
set (DEBUGGER_ENABLED 0)
set (COMPILER_LIBRARY_NAME ${PROJECT_NAME})

if (CURSES_FOUND)
    set (
        DEBUGGER_SOURCES
        debugger/window.cpp debugger/window.h
        debugger/environment.cpp debugger/environment.h
        debugger/stack_window.cpp debugger/stack_window.h
        debugger/header_window.cpp debugger/header_window.h
        debugger/footer_window.cpp debugger/footer_window.h
        debugger/output_window.cpp debugger/output_window.h
        debugger/memory_window.cpp debugger/memory_window.h
        debugger/errors_window.cpp debugger/errors_window.h
        debugger/debugger_types.cpp debugger/debugger_types.h
        debugger/command_window.cpp debugger/command_window.h
        debugger/assembly_window.cpp debugger/assembly_window.h
        debugger/registers_window.cpp debugger/registers_window.h)
    set (DEBUGGER_ENABLED 1)
    message (STATUS "DEBUGGER_ENABLED = ${DEBUGGER_ENABLED}")
endif()

configure_file (
    "${PROJECT_SOURCE_DIR}/configure.in.h"
    "${PROJECT_BINARY_DIR}/configure.h"
)

include_directories(
    ${PROJECT_SOURCE_DIR}
    ${PROJECT_BINARY_DIR}
)

add_library(
    ${PROJECT_NAME} SHARED

    include/basecode/compiler.h

    configure.in.h

    common/defer.h
    common/bytes.h
    common/result.h
    common/result_message.h
    common/rune.cpp common/rune.h
    common/id_pool.cpp common/id_pool.h
    common/colorizer.cpp common/colorizer.h
    common/source_file.cpp common/source_file.h
    common/memory_pool.cpp common/memory_pool.h
    common/hex_formatter.cpp common/hex_formatter.h
    common/string_support.cpp common/string_support.h
    common/source_location.cpp common/source_location.h
    common/graphviz_formatter.cpp common/graphviz_formatter.h

    parser/ast.cpp parser/ast.h
    parser/lexer.cpp parser/lexer.h
    parser/token.cpp parser/token.h
    parser/parser.cpp parser/parser.h
    parser/token_pool.cpp parser/token_pool.h
    parser/ast_formatter.cpp parser/ast_formatter.h

    vm/ffi.cpp vm/ffi.h
    vm/terp.cpp vm/terp.h
    vm/label.cpp vm/label.h
    vm/symbol.cpp vm/symbol.h
    vm/vm_types.h vm/vm_types.cpp
    vm/assembler.cpp vm/assembler.h
    vm/label_map.cpp vm/label_map.h
    vm/basic_block.cpp vm/basic_block.h
    vm/basic_block_map.cpp vm/basic_block_map.h
    vm/assembly_parser.cpp vm/assembly_parser.h
    vm/assembly_listing.cpp vm/assembly_listing.h
    vm/default_allocator.cpp vm/default_allocator.h
    vm/register_allocator.cpp vm/register_allocator.h

    ${DEBUGGER_SOURCES}

    compiler/elements.h

    compiler/api/dom.cpp
    compiler/api/wrappers.cpp
    compiler/api/kernel.cpp compiler/api/kernel.h

    compiler/session.cpp compiler/session.h
    compiler/element_map.cpp compiler/element_map.h
    compiler/variable_map.cpp compiler/variable_map.h
    compiler/elements/type.cpp compiler/elements/type.h
    compiler/elements/cast.cpp compiler/elements/cast.h
    compiler/scope_manager.cpp compiler/scope_manager.h
    compiler/ast_evaluator.cpp compiler/ast_evaluator.h
    compiler/elements/with.cpp compiler/elements/with.h
    compiler/elements/yield.cpp compiler/elements/yield.h
    compiler/compiler_types.cpp compiler/compiler_types.h
    compiler/elements/field.cpp compiler/elements/field.h
    compiler/elements/block.cpp compiler/elements/block.h
    compiler/elements/label.cpp compiler/elements/label.h
    compiler/element_builder.cpp compiler/element_builder.h
    compiler/elements/import.cpp compiler/elements/import.h
    compiler/elements/module.cpp compiler/elements/module.h
    compiler/elements/comment.cpp compiler/elements/comment.h
    compiler/elements/element.cpp compiler/elements/element.h
    compiler/elements/program.cpp compiler/elements/program.h
    compiler/type_name_builder.cpp compiler/type_name_builder.h
    compiler/byte_code_emitter.cpp compiler/byte_code_emitter.h
    compiler/string_intern_map.cpp compiler/string_intern_map.h
    compiler/elements/rune_type.cpp compiler/elements/rune_type.h
    compiler/elements/intrinsic.cpp compiler/elements/intrinsic.h
    compiler/elements/transmute.cpp compiler/elements/transmute.h
    compiler/elements/bool_type.cpp compiler/elements/bool_type.h
    compiler/elements/attribute.cpp compiler/elements/attribute.h
    compiler/elements/directive.cpp compiler/elements/directive.h
    compiler/elements/statement.cpp compiler/elements/statement.h
    compiler/code_dom_formatter.cpp compiler/code_dom_formatter.h
    compiler/elements/raw_block.cpp compiler/elements/raw_block.h
    compiler/elements/assignment.cpp compiler/elements/assignment.h
    compiler/elements/if_element.cpp compiler/elements/if_element.h
    compiler/elements/identifier.cpp compiler/elements/identifier.h
    compiler/elements/expression.cpp compiler/elements/expression.h
    compiler/elements/array_type.cpp compiler/elements/array_type.h
    compiler/elements/tuple_type.cpp compiler/elements/tuple_type.h
    compiler/elements/family_type.cpp compiler/elements/family_type.h
    compiler/elements/for_element.cpp compiler/elements/for_element.h
    compiler/elements/nil_literal.cpp compiler/elements/nil_literal.h
    compiler/elements/initializer.cpp compiler/elements/initializer.h
    compiler/elements/module_type.cpp compiler/elements/module_type.h
    compiler/elements/declaration.cpp compiler/elements/declaration.h
    compiler/elements/fallthrough.cpp compiler/elements/fallthrough.h
    compiler/elements/generic_type.cpp compiler/elements/generic_type.h
    compiler/elements/type_literal.cpp compiler/elements/type_literal.h
    compiler/elements/numeric_type.cpp compiler/elements/numeric_type.h
    compiler/elements/unknown_type.cpp compiler/elements/unknown_type.h
    compiler/elements/pointer_type.cpp compiler/elements/pointer_type.h
    compiler/elements/case_element.cpp compiler/elements/case_element.h
    compiler/elements/if_directive.cpp compiler/elements/if_directive.h
    compiler/elements/operator_base.cpp compiler/elements/operator_base.h
    compiler/elements/float_literal.cpp compiler/elements/float_literal.h
    compiler/elements/element_types.cpp compiler/elements/element_types.h
    compiler/elements/argument_list.cpp compiler/elements/argument_list.h
    compiler/elements/while_element.cpp compiler/elements/while_element.h
    compiler/elements/break_element.cpp compiler/elements/break_element.h
    compiler/elements/defer_element.cpp compiler/elements/defer_element.h
    compiler/elements/argument_pair.cpp compiler/elements/argument_pair.h
    compiler/elements/run_directive.cpp compiler/elements/run_directive.h
    compiler/elements/type_directive.cpp compiler/elements/type_directive.h
    compiler/elements/switch_element.cpp compiler/elements/switch_element.h
    compiler/elements/copy_intrinsic.cpp compiler/elements/copy_intrinsic.h
    compiler/elements/fill_intrinsic.cpp compiler/elements/fill_intrinsic.h
    compiler/elements/free_intrinsic.cpp compiler/elements/free_intrinsic.h
    compiler/elements/symbol_element.cpp compiler/elements/symbol_element.h
    compiler/elements/namespace_type.cpp compiler/elements/namespace_type.h
    compiler/elements/composite_type.cpp compiler/elements/composite_type.h
    compiler/elements/procedure_type.cpp compiler/elements/procedure_type.h
    compiler/elements/string_literal.cpp compiler/elements/string_literal.h
    compiler/elements/unary_operator.cpp compiler/elements/unary_operator.h
    compiler/elements/return_element.cpp compiler/elements/return_element.h
    compiler/elements/procedure_call.cpp compiler/elements/procedure_call.h
    compiler/elements/assembly_label.cpp compiler/elements/assembly_label.h
    compiler/elements/type_reference.cpp compiler/elements/type_reference.h
    compiler/elements/label_reference.cpp compiler/elements/label_reference.h
    compiler/elements/spread_operator.cpp compiler/elements/spread_operator.h
    compiler/elements/alloc_intrinsic.cpp compiler/elements/alloc_intrinsic.h
    compiler/elements/integer_literal.cpp compiler/elements/integer_literal.h
    compiler/elements/binary_operator.cpp compiler/elements/binary_operator.h
    compiler/elements/boolean_literal.cpp compiler/elements/boolean_literal.h
    compiler/elements/range_intrinsic.cpp compiler/elements/range_intrinsic.h
    compiler/elements/assert_directive.cpp compiler/elements/assert_directive.h
    compiler/elements/continue_element.cpp compiler/elements/continue_element.h
    compiler/elements/module_reference.cpp compiler/elements/module_reference.h
    compiler/elements/character_literal.cpp compiler/elements/character_literal.h
    compiler/elements/type_of_intrinsic.cpp compiler/elements/type_of_intrinsic.h
    compiler/elements/namespace_element.cpp compiler/elements/namespace_element.h
    compiler/elements/size_of_intrinsic.cpp compiler/elements/size_of_intrinsic.h
    compiler/elements/foreign_directive.cpp compiler/elements/foreign_directive.h
    compiler/elements/value_sink_literal.cpp compiler/elements/value_sink_literal.h
    compiler/elements/assembly_directive.cpp compiler/elements/assembly_directive.h
    compiler/elements/align_of_intrinsic.cpp compiler/elements/align_of_intrinsic.h
    compiler/elements/intrinsic_directive.cpp compiler/elements/intrinsic_directive.h
    compiler/elements/core_type_directive.cpp compiler/elements/core_type_directive.h
    compiler/elements/length_of_intrinsic.cpp compiler/elements/length_of_intrinsic.h
    compiler/elements/address_of_intrinsic.cpp compiler/elements/address_of_intrinsic.h
    compiler/elements/identifier_reference.cpp compiler/elements/identifier_reference.h
    compiler/elements/uninitialized_literal.cpp compiler/elements/uninitialized_literal.h
    compiler/elements/assembly_literal_label.cpp compiler/elements/assembly_literal_label.h)
target_link_libraries(
    ${PROJECT_NAME}
    fmt-header-only
    dyncall_s
    dynload_s
    boost-system
    boost-filesystem
    utf8proc
    ${DL_LIBRARY}
    ${CURSES_LIBRARIES})
