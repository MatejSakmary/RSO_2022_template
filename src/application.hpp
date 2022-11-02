#pragma once

#include "window.hpp"
#include "types.hpp"

struct Application
{
    public:
        Application();
        ~Application();

        void run_loop();

    private:
        AppWindow window;

        void init_window();
        void mouse_pos_callback(f64 x, f64 y);
        void mouse_button_callback(i32 button, i32 action, i32 mods);
        void key_callback(i32 key, i32 code, i32 action, i32 mods);
        void window_resized_callback(i32 width, i32 height);
};