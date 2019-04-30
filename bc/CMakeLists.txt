cmake_minimum_required(VERSION 3.10)
project(bc)

include_directories(
    ${PROJECT_SOURCE_DIR}
    ${PROJECT_SOURCE_DIR}/../basecode
    ${PROJECT_SOURCE_DIR}/../basecode/include
)

add_executable(
    ${PROJECT_NAME}
    main.cpp
    ../ext/ya_getopt-1.0.0/ya_getopt.c
)
target_link_libraries(
    ${PROJECT_NAME}
    basecode
    fmt-header-only
    boost-system
    boost-filesystem
)
