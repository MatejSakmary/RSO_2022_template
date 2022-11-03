#pragma once

#include <functional>

#include "types.hpp"

#include <GLFW/glfw3.h>
#if defined(_WIN32)
#define GLFW_EXPOSE_NATIVE_WIN32
#define GLFW_NATIVE_INCLUDE_NONE
#define WIN32_LEAN_AND_MEAN
#define NOMINMAX
#include <dwmapi.h>
#ifndef DWMWA_USE_IMMERSIVE_DARK_MODE
#define DWMWA_USE_IMMERSIVE_DARK_MODE 20
#endif
#elif defined(__linux__)
#define GLFW_EXPOSE_NATIVE_X11
#define GLFW_EXPOSE_NATIVE_WAYLAND
#endif
#include <GLFW/glfw3native.h>


struct WindowVTable
{
    std::function<void(f64, f64)> mouse_pos_callback;
    std::function<void(i32, i32, i32)> mouse_button_callback;
    std::function<void(i32, i32, i32, i32)> key_callback;
    std::function<void(i32, i32)> window_resized_callback;
};

struct AppWindow
{
    public:
        AppWindow(u32 width, u32 height, WindowVTable callbacks) : v_table{callbacks}
        {
            glfwInit();
            glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);
            glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
            glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
            glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_COMPAT_PROFILE);
            window = glfwCreateWindow(width, height, "Vulkan", nullptr, nullptr);
            glfwMakeContextCurrent(window);
            glfwSetWindowUserPointer(window, &v_table);
            glfwSetCursorPosCallback( 
                window,
                [](GLFWwindow *window, f64 x, f64 y)
                { 
                    auto &v_table = *reinterpret_cast<WindowVTable *>(glfwGetWindowUserPointer(window));
                    v_table.mouse_pos_callback(x, y); 
                }
            );
            glfwSetMouseButtonCallback(
                window,
                [](GLFWwindow *window, i32 button, i32 action, i32 mods)
                {
                    auto &v_table = *reinterpret_cast<WindowVTable *>(glfwGetWindowUserPointer(window));
                    v_table.mouse_button_callback(button, action, mods);
                }
            );
            glfwSetKeyCallback(
                window,
                [](GLFWwindow *window, i32 key, i32 code, i32 action, i32 mods)
                {
                    auto &v_table = *reinterpret_cast<WindowVTable *>(glfwGetWindowUserPointer(window));
                    v_table.key_callback(key, code, action, mods);
                }
            );
            glfwSetFramebufferSizeCallback( 
                window, 
                [](GLFWwindow *window, i32 x, i32 y)
                { 
                    auto &v_table = *reinterpret_cast<WindowVTable *>(glfwGetWindowUserPointer(window));
                    v_table.window_resized_callback(x, y); 
                }
            );
        }

        i32 get_key_state(i32 key) { return glfwGetKey(window, key); }
        void set_window_close() { glfwSetWindowShouldClose(window, true); }
        void set_input_mode(i32 mode, i32 value) { glfwSetInputMode(window, mode, value); }
        bool get_window_should_close() { return glfwWindowShouldClose(window); }
        void swap_buffers() { glfwSwapBuffers(window); }

        ~AppWindow()
        {
            glfwDestroyWindow(window);
            glfwTerminate();
        }
    private:
        WindowVTable v_table;
        GLFWwindow* window;
};