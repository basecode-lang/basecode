cmake_minimum_required(VERSION 3.15)
project(vm-workbench)

set(ARTIFACT_NAME "workbench")
set(VERSION_MAJOR 0)
set(VERSION_MINOR 1)
set(PRODUCT_NAME  "Basecode Virtual Machine Workbench")

configure_file(
        configure.in.h
        "${CMAKE_BINARY_DIR}/include/basecode/workbench/configure.h"
)

include_directories(
        ${CMAKE_CURRENT_SOURCE_DIR}
        ${CMAKE_SOURCE_DIR}/ext/imgui-docking
        ${CMAKE_SOURCE_DIR}/ext/gl3w-1.0.0/include
)

set(FRAMEWORKS "")
if (APPLE)
    set(
            FRAMEWORKS
            "-framework OpenGL"
            "-framework Cocoa"
            "-framework IOKit"
            "-framework CoreVideo"
            "-framework Quartz"
    )
endif()

add_executable(
        ${ARTIFACT_NAME}

        main.cpp

        ${CMAKE_SOURCE_DIR}/ext/gl3w-1.0.0/src/gl3w.c
        ${CMAKE_SOURCE_DIR}/ext/imgui-docking/imconfig.h
        ${CMAKE_SOURCE_DIR}/ext/imgui-docking/imgui_demo.cpp
        ${CMAKE_SOURCE_DIR}/ext/imgui-docking/imgui_draw.cpp
        ${CMAKE_SOURCE_DIR}/ext/imgui-docking/imgui_internal.h
        ${CMAKE_SOURCE_DIR}/ext/imgui-docking/imstb_rectpack.h
        ${CMAKE_SOURCE_DIR}/ext/imgui-docking/imstb_textedit.h
        ${CMAKE_SOURCE_DIR}/ext/imgui-docking/imstb_truetype.h
        ${CMAKE_SOURCE_DIR}/ext/imgui-docking/imgui_widgets.cpp
        ${CMAKE_SOURCE_DIR}/ext/imgui-docking/imgui.cpp ${CMAKE_SOURCE_DIR}/ext/imgui-docking/imgui.h
        ${CMAKE_SOURCE_DIR}/ext/imgui-docking/examples/imgui_impl_glfw.cpp ${CMAKE_SOURCE_DIR}/ext/imgui-docking/examples/imgui_impl_glfw.h
        ${CMAKE_SOURCE_DIR}/ext/imgui-docking/examples/imgui_impl_opengl3.cpp ${CMAKE_SOURCE_DIR}/ext/imgui-docking/examples/imgui_impl_opengl3.h
        ${CMAKE_SOURCE_DIR}/ext/imgui-docking/misc/freetype/imgui_freetype.cpp ${CMAKE_SOURCE_DIR}/ext/imgui-docking/misc/freetype/imgui_freetype.h
)
target_link_libraries(
        ${ARTIFACT_NAME}

        vm
        freetype
        glfw
        fmt-header-only
        core
        ${FRAMEWORKS}
)