cmake_minimum_required(VERSION 3.15)
project(vm-generator)

option(PROFILE "Enable Tracy profiler" OFF)

if (PROFILE)
    add_definitions(-DTRACY_ENABLE)
endif()

set(ARTIFACT_NAME "vmgen")
set(VERSION_MAJOR 0)
set(VERSION_MINOR 1)
set(PRODUCT_NAME  "Basecode Virtual Machine Instruction Generator")

configure_file(
        configure.in.h
        "${CMAKE_BINARY_DIR}/include/basecode/vmgen/configure.h"
)

add_executable(
        ${ARTIFACT_NAME}

        dsl/types.h dsl/ast.cpp dsl/lexer.cpp dsl/parser.cpp

        main.cpp
        instructions.ig
        ${CMAKE_SOURCE_DIR}/ext/tracy-master/TracyClient.cpp
)
target_link_libraries(
        ${ARTIFACT_NAME}

        fmt-header-only
        core
        ${DL_LIBRARY}
)

add_custom_command(
        TARGET ${ARTIFACT_NAME} POST_BUILD

        COMMAND ${CMAKE_COMMAND} -E make_directory ${CMAKE_BINARY_DIR}/assets
        COMMAND ${CMAKE_COMMAND} -E copy ${CMAKE_CURRENT_SOURCE_DIR}/instructions.ig ${CMAKE_BINARY_DIR}/assets/instructions.ig
)