cmake_minimum_required(VERSION 3.15)
project(vm-lib)

set(ARTIFACT_NAME "vm")
set(VERSION_MAJOR 0)
set(VERSION_MINOR 1)
set(PRODUCT_NAME  "Basecode Virtual Machine Library")

configure_file(
        configure.in.h
        "${CMAKE_BINARY_DIR}/include/basecode/vm/configure.h"
)

add_library(
        ${ARTIFACT_NAME}

        types.cpp types.h

        ${CMAKE_SOURCE_DIR}/ext/tracy-master/TracyClient.cpp
)
target_link_libraries(
        ${ARTIFACT_NAME}

        asmjit
        dyncall_s
        dynload_s
        core
        ${DL_LIBRARY}
)