cmake_minimum_required(VERSION 3.15)
project(bcc)

message(STATUS "--------------- ${PROJECT_NAME} build configuration ---------------")

include_directories(
        ${PROJECT_SOURCE_DIR}
        ${PROJECT_SOURCE_DIR}/../basecode
        ${PROJECT_SOURCE_DIR}/../ext/ya_getopt-master
)

add_executable(
        ${PROJECT_NAME}

        ../ext/ya_getopt-master/ya_getopt.c ../ext/ya_getopt-master/ya_getopt.h

        main.cpp)

target_link_libraries(
        ${PROJECT_NAME}

        basecode
)