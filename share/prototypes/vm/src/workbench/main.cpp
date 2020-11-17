// ----------------------------------------------------------------------------
// ____                               _
// |  _\                             | |
// | |_)| __ _ ___  ___  ___ ___   __| | ___ TM
// |  _< / _` / __|/ _ \/ __/ _ \ / _` |/ _ \
// | |_)| (_| \__ \  __/ (_| (_) | (_| |  __/
// |____/\__,_|___/\___|\___\___/ \__,_|\___|
//
// V I R T U A L  M A C H I N E  P R O J E C T
//
// Copyright (C) 2020 Jeff Panici
// All rights reserved.
//
// This software source file is licensed under the terms of MIT license.
// For details, please read the LICENSE file.
//
// ----------------------------------------------------------------------------

#include <thread>
#include <imgui.h>
#include <GL/gl3w.h>
#include <GLFW/glfw3.h>
#include <basecode/core/defer.h>
#include <examples/imgui_impl_glfw.h>
#include <examples/imgui_impl_opengl3.h>
#include <basecode/core/memory/system.h>
#include <basecode/core/timers/system.h>
#include <basecode/core/graphics/types.h>
#include <basecode/workbench/configure.h>
#include <basecode/core/profiler/system.h>

using namespace basecode;

static float s_dpi_scale = 1.0f;
static graphics::window_t s_window{};
static graphics::vector4_t s_clear_color = {0.45f, 0.55f, 0.60f, 1.00f};

static void window_close_callback(GLFWwindow* window) {
}

static void window_focus_callback(GLFWwindow* window, int focused) {
}

static void window_iconify_callback(GLFWwindow* window, int iconified) {
}

static void window_pos_callback(GLFWwindow* window, int x, int y) {
}

static void window_size_callback(GLFWwindow* window, int width, int height) {
}

static void window_maximize_callback(GLFWwindow* window, int maximized) {
}

static void key_callback(GLFWwindow* window, int key, int scan_code, int action, int mods) {
}

static void error_callback(int error, const char* description) {
}

int main(int argc, char** argv) {
    memory::initialize();
    defer(memory::shutdown());

    context::context_t ctx{
        .allocator = memory::default_allocator()
    };
    context::push(&ctx);
    defer(context::pop());

    {
        auto rc = profiler::initialize();
        if (rc != profiler::init_result_t::ok) {
            // XXX:
            return 1;
        }
    }
    defer(profiler::shutdown());

    {
        auto rc = timers::initialize();
        if (rc != timers::init_result_t::ok) {
            // XXX:
            return 1;
        }
    }
    defer(timers::shutdown());

    glfwSetErrorCallback(error_callback);
    if (!glfwInit())
        return 1;

    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 2);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
#if __APPLE__
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
#endif
    glfwWindowHint(GLFW_MAXIMIZED, s_window.maximized != 0);

    s_window.window = glfwCreateWindow(
        s_window.width,
        s_window.height,
        PRODUCT_NAME,
        nullptr,
        nullptr);
    if (!s_window.window)
        return 1;

    glfwSetWindowUserPointer(s_window.window, &s_window);
    glfwSetWindowSizeLimits(
        s_window.window,
        s_window.min_width,
        s_window.min_height,
        GLFW_DONT_CARE,
        GLFW_DONT_CARE);

    glfwSetKeyCallback(s_window.window, key_callback);
    glfwSetWindowPosCallback(s_window.window, window_pos_callback);
    glfwSetWindowSizeCallback(s_window.window, window_size_callback);
    glfwSetWindowFocusCallback(s_window.window, window_focus_callback);
    glfwSetWindowCloseCallback(s_window.window, window_close_callback);
    glfwSetWindowIconifyCallback(s_window.window, window_iconify_callback);
    glfwSetWindowMaximizeCallback(s_window.window, window_maximize_callback);

    glfwMakeContextCurrent(s_window.window);
    glfwSwapInterval(1);

    if (gl3wInit() != 0)
        return 1;

    IMGUI_CHECKVERSION();
    ImGui::CreateContext();

    auto& io = ImGui::GetIO();
    //io.IniFilename = _ini_path.string().c_str();
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;
    io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;
    io.ConfigFlags |= ImGuiConfigFlags_ViewportsEnable;

    io.FontDefault = io.Fonts->AddFontDefault();

    ImGui::StyleColorsDark();

    auto& style = ImGui::GetStyle();
    style.WindowBorderSize = 1.f * s_dpi_scale;
    style.FrameBorderSize = 1.f * s_dpi_scale;
    style.WindowRounding = 0.0f;

    style.Colors[ImGuiCol_WindowBg].w = 1.0f;

    ImGui_ImplGlfw_InitForOpenGL(s_window.window, true);
    ImGui_ImplOpenGL3_Init("#version 150");

    int dw{};
    int dh{};

    while (!glfwWindowShouldClose(s_window.window)) {
        glfwPollEvents();

        if (glfwGetWindowAttrib(s_window.window, GLFW_ICONIFIED)) {
            std::this_thread::yield();
            continue;
        }

        timers::update();

        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();

        glfwGetFramebufferSize(s_window.window, &dw, &dh);
        glViewport(0, 0, dw, dh);
        glClearColor(
            s_clear_color.x,
            s_clear_color.y,
            s_clear_color.z,
            s_clear_color.w);
        glClear(GL_COLOR_BUFFER_BIT);
        ImGui::Render();
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

        auto backup_current_context = glfwGetCurrentContext();
        ImGui::UpdatePlatformWindows();
        ImGui::RenderPlatformWindowsDefault();
        glfwMakeContextCurrent(backup_current_context);

        glfwSwapBuffers(s_window.window);

        if (!glfwGetWindowAttrib(s_window.window, GLFW_FOCUSED)) {
            std::this_thread::yield();
        }
    }

    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();

    glfwDestroyWindow(s_window.window);
    glfwTerminate();

    return 0;
}
