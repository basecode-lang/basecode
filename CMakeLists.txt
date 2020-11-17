cmake_minimum_required(VERSION 3.16)
project(basecode C CXX ASM)

set(CMAKE_CXX_STANDARD 20)
if(${CMAKE_SYSTEM_NAME} STREQUAL "Linux")
    set(LINUX TRUE)
endif()

set(CMAKE_CXX_STANDARD_REQUIRED     ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS   ON)
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY  ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY  ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY  ${CMAKE_BINARY_DIR}/bin)

# common commands in a shorter form
set(MKDIR COMMAND ${CMAKE_COMMAND} -E make_directory)
set(CP    COMMAND ${CMAKE_COMMAND} -E copy)
set(CPR   COMMAND ${CMAKE_COMMAND} -E copy_directory)
set(CP_IF COMMAND ${CMAKE_COMMAND} -E copy_if_different)
set(LN    COMMAND ${CMAKE_COMMAND} -E create_symlink)

find_program(CCACHE_FOUND ccache)
if (CCACHE_FOUND)
    set_property(GLOBAL PROPERTY RULE_LAUNCH_COMPILE ccache)
    set_property(GLOBAL PROPERTY RULE_LAUNCH_LINK ccache)
    message(STATUS "${PROJECT_NAME} using ccache.")
endif(CCACHE_FOUND)

if (${CMAKE_SYSTEM_NAME} MATCHES "Darwin")
    set(CMAKE_C_ARCHIVE_CREATE   "<CMAKE_AR> Scr <TARGET> <LINK_FLAGS> <OBJECTS>")
    set(CMAKE_CXX_ARCHIVE_CREATE "<CMAKE_AR> Scr <TARGET> <LINK_FLAGS> <OBJECTS>")
    set(CMAKE_C_ARCHIVE_FINISH   "<CMAKE_RANLIB> -no_warning_for_no_symbols -c <TARGET>")
    set(CMAKE_CXX_ARCHIVE_FINISH "<CMAKE_RANLIB> -no_warning_for_no_symbols -c <TARGET>")
endif()

if (MSVC)
    set(
        CXX_FLAGS
        /W3
        /EHsc
        -Qunused-arguments
        -Wno-c++98-compat
        -Wno-c++98-compat-pedantic
        -Wno-signed-enum-bitfield
        -Wno-nonportable-system-include-path
        -Wno-newline-eof
        -Wno-deprecated-declarations
        -Wno-documentation-unknown-command
        -Wno-c99-designator
    )
    add_compile_options(
        "$<$<COMPILE_LANGUAGE:C,CXX>:${CXX_FLAGS}>"
    )
    set(
        CXX_DEFINES
        NOMINMAX
        _CRT_SECURE_NO_WARNINGS
        _CRT_DEPRECATED_NO_WARNINGS
        _WINSOCK_DEPRECATED_NO_WARNINGS
    )
    add_compile_definitions(
        "$<$<COMPILE_LANGUAGE:C,CXX>:${CXX_DEFINES}>"
    )
else ()
    set(
        CXX_FLAGS
        -Wall
        -Wno-string-plus-int
        -Wno-signed-enum-bitfield
        -Wno-cast-qual
        -Wno-auto-var-id
        -Wno-unused-private-field
        -Wno-unknown-pragmas
        -Wno-inconsistent-missing-override
        -Wno-c99-designator
        -Wno-deprecated-declarations
    )

    set(CLANG_CXX_FLAGS -fcolor-diagnostics)

    set(GCC_CXX_FLAGS -fdiagnostics-color=always -Wno-parentheses -Wno-sequence-point)

    add_compile_options(
        "$<$<COMPILE_LANGUAGE:C,CXX>:${CXX_FLAGS}>"
        "$<$<COMPILE_LANG_AND_ID:CXX,GNU>:${GCC_CXX_FLAGS}>"
        "$<$<COMPILE_LANG_AND_ID:CXX,Clang>:${CLANG_CXX_FLAGS}>"
    )
endif()

# -----------------------------------------------------------------------------
#
# Dependencies
#
# -----------------------------------------------------------------------------

# basecode-foundation
add_subdirectory(ext/foundation EXCLUDE_FROM_ALL)

if (WIN32)
    include_directories(include/platform/win32)
endif()

# -----------------------------------------------------------------------------
#
# Project
#
# -----------------------------------------------------------------------------

set(INC_DIR_BASE    "${PROJECT_SOURCE_DIR}/include")
set(PRJ_DIR_BASE    ${PROJECT_SOURCE_DIR})

add_subdirectory(src/libcompiler)
add_subdirectory(src/libcompiler-test)
