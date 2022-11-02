#include "application.hpp"

#include <functional>
#include <iostream>

void Application::mouse_pos_callback(f64 x, f64 y)
{
    std::cout << "mouse pos callback" << std::endl;
    return;
}

void Application::mouse_button_callback(i32 button, i32 action, i32 mods)
{
    std::cout << "mouse button callback" << std::endl;
    return;
}

void Application::key_callback(i32 key, i32 code, i32 action, i32 mods)
{
    std::cout << "key callback" << std::endl;
    return;
}

void Application::window_resized_callback(i32 width, i32 height)
{
    std::cout << "window resized callback" << std::endl;
    return;
}

Application::Application() :
    window
    (
        1080, 720,
        WindowVTable {
            .mouse_pos_callback = std::bind(
                &Application::mouse_pos_callback, this,
                std::placeholders::_1,
                std::placeholders::_2),
            .mouse_button_callback = std::bind(
                &Application::mouse_button_callback, this,
                std::placeholders::_1,
                std::placeholders::_2,
                std::placeholders::_3),
            .key_callback = std::bind(
                &Application::key_callback, this,
                std::placeholders::_1,
                std::placeholders::_2,
                std::placeholders::_2,
                std::placeholders::_4),
            .window_resized_callback = std::bind(
                &Application::window_resized_callback, this,
                std::placeholders::_1,
                std::placeholders::_2)
        }
    )
{ 
}

Application::~Application()
{
}

void Application::run_loop()
{
    while(!window.get_window_should_close())
    {
        glfwPollEvents();
    }
}
