cmake_minimum_required(VERSION 3.10)
project(bc)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

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

if ("${CMAKE_SYSTEM_NAME}" MATCHES "FreeBSD")
    add_definitions(-D__USE_ISOC99)
endif()

if ("${CMAKE_CXX_COMPILER_ID}" MATCHES "Clang")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Wno-unused-private-field -Wno-unknown-pragmas -Wno-inconsistent-missing-override" CACHE STRING "compile flags" FORCE)
    message(STATUS "${PROJECT_NAME} using clang flags: ${CMAKE_CXX_FLAGS}")
elseif ("${CMAKE_CXX_COMPILER_ID}" MATCHES "GNU")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Wno-unknown-pragmas -Wno-parentheses" CACHE STRING "compile flags" FORCE)
    message(STATUS "${PROJECT_NAME} using gcc flags: ${CMAKE_CXX_FLAGS}")
else ()
    message(FATAL_ERROR "${PROJECT_NAME} unknown compiler: ${CMAKE_CXX_COMPILER_ID}")
endif()

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
