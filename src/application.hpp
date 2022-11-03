#pragma once

#include "window.hpp"
#include "types.hpp"
#include "raytracing_backend/scene.hpp"
#include "raytracing_backend/camera.hpp"
#include "raytracing_backend/raytracer.hpp"
#include "raytracing_backend/material.hpp"
#include "raytracing_backend/intersectable.hpp"


struct Application
{
    public:
        const u32vec2 WINDOW_DIMENSIONS = {1080, 720};
        Application();
        ~Application();

        void run_loop();

    private:
        AppWindow window;
        Scene scene;
        Raytracer raytracer;

        void init_window();
        void mouse_pos_callback(f64 x, f64 y);
        void mouse_button_callback(i32 button, i32 action, i32 mods);
        void key_callback(i32 key, i32 code, i32 action, i32 mods);
        void window_resized_callback(i32 width, i32 height);

        Scene create_default_scene();
};