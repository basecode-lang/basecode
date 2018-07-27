cmake_minimum_required(VERSION 3.11)
project(bac)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

if ("${CMAKE_CXX_COMPILER_ID}" MATCHES "Clang")
    set(CMAKE_CXX_FLAGS "-Wall -Wno-unused-private-field -Wno-unknown-pragmas -Wno-inconsistent-missing-override" CACHE STRING "compile flags" FORCE)
    message(STATUS "bac using clang flags: ${CMAKE_CXX_FLAGS}")
elseif ("${CMAKE_CXX_COMPILER_ID}" MATCHES "GNU")
    set(CMAKE_CXX_FLAGS "-lstdc++fs -Wall -Wno-unknown-pragmas" CACHE STRING "compile flags" FORCE)
    message(STATUS "bac using gcc flags: ${CMAKE_CXX_FLAGS}")
else ()
    message(STATUS "bac unknown compiler: ${CMAKE_CXX_COMPILER_ID}")
endif()

include_directories(
    ${PROJECT_SOURCE_DIR}
    ${PROJECT_SOURCE_DIR}/../alpha-core
)

add_executable(
    ${PROJECT_NAME}
    main.cpp
    ya_getopt.c ya_getopt.h
)
target_link_libraries(
    ${PROJECT_NAME}
    alpha-core
    fmt-header-only
    boost-system
    boost-filesystem
)
