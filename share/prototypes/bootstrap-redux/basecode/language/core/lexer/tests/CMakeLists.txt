cmake_minimum_required(VERSION 3.15)
project(basecode-language-core-lexer-tests)

add_executable(${PROJECT_NAME} main.cpp lexer_tests.cpp)
target_link_libraries(${PROJECT_NAME} basecode)

enable_testing()
add_test(NAME ${PROJECT_NAME} COMMAND ${PROJECT_NAME})