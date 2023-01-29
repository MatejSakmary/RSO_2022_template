#include "application.hpp"

#include <functional>
#include <iostream>
#include <string>
#include <array>

#include <omp.h>

void Application::mouse_pos_callback(f64 x, f64 y)
{
    return;
}

void Application::mouse_button_callback(i32 button, i32 action, i32 mods)
{
    return;
}

void Application::key_callback(i32 key, i32 code, i32 action, i32 mods)
{
    if(key == GLFW_KEY_L && action == GLFW_PRESS)
    {
        std::cout << "raytracing scene - light sampling" << std::endl;
        raytracer.set_sample_ratio(1.0f);
        raytracer.trace_scene(&scene, {
            .samples = 100,
            .iterations = 10,
            .method = TraceMethod::LIGHT_SOURCE
        });
    }
    if(key == GLFW_KEY_B && action == GLFW_PRESS)
    {
        std::cout << "raytracing scene - brdf sampling" << std::endl;
        scene.use_env_map = true;
        raytracer.set_sample_ratio(0.0f);
        raytracer.trace_scene(&scene, {
            .samples = 80,
            .iterations = 1,
            .method = TraceMethod::BRDF
        });
    }
    if(key == GLFW_KEY_M && action == GLFW_PRESS)
    {
        std::cout << "raytracing scene - multiimportance sampling" << std::endl;
        raytracer.set_sample_ratio(0.5f);
        raytracer.trace_scene(&scene, {
            .samples = 100,
            .iterations = 10,
            .method = TraceMethod::MULTI_IMPORTANCE
        });
    }
    if(key == GLFW_KEY_W && action == GLFW_PRESS)
    {
        std::cout << "raytracing scene - multiimportance weighed sampling" << std::endl;
        raytracer.set_sample_ratio(0.5f);
        raytracer.trace_scene(&scene, {
            .samples = 100,
            .iterations = 10,
            .method = TraceMethod::MULTI_IMPORTANCE_WEIGHTS
        });
    }
    else if(key == GLFW_KEY_RIGHT && action == GLFW_PRESS)
    {
        image_idx = (image_idx + 1) % 11;
        load_env_map_image();
    }
    else if(key == GLFW_KEY_LEFT && action == GLFW_PRESS)
    {
        image_idx = glm::min(11u, (image_idx - 1) % 11);
        load_env_map_image();
    }
    else if(key == GLFW_KEY_E && action == GLFW_PRESS)
    {
        show_env_map = !show_env_map;
    }
    return;
}

void Application::window_resized_callback(i32 width, i32 height)
{
    return;
}

Application::Application() :
    window
    (
        WINDOW_DIMENSIONS.x, WINDOW_DIMENSIONS.y,
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
                std::placeholders::_3,
                std::placeholders::_4),
            .window_resized_callback = std::bind(
                &Application::window_resized_callback, this,
                std::placeholders::_1,
                std::placeholders::_2)
        }
    ),
    scene{create_default_scene()},
    raytracer{WINDOW_DIMENSIONS},
    image_idx{0},
    show_env_map{false}
{ 
    load_env_map_image();
}

Application::~Application()
{
}

void Application::load_env_map_image()
{
    auto path_from_index = [](int index) -> std::string
    {
        std::array<std::string, 11> img_num { "001", "004", "007", "010", "011", "013", "015", "016", "020", "023", "024" };
        return "assets/textures/EM/raw" + img_num[index] + ".hdr";
    };
    load_hdr_image(path_from_index(image_idx), scene.env_map.image, scene.env_map.width, scene.env_map.height);
    scene.env_map.init();
    std::cout << "[Application::load_env_map_image()] Image " << image_idx <<  " loaded!" << std::endl;
}

Scene Application::create_default_scene()
{
    Scene scene = Scene(Camera::CameraInfo{
        .origin = {0.0, 6.0, 18.0},
        .look_at = {0.0, 0.0, 0.0},
        .up = {0.0, 1.0, 0.0},
        .fov = 35.0 * M_PI / 180.0
    });

    Material::MaterialCreateInfo mat_table_info = {
        .Le = {0.0, 0.0, 0.0},
        .diffuse_albedo = {0.8, 0.8, 0.8},
        .specular_albedo = {0.2, 0.2, 0.2},
        .shininess = 500.0
    };

    Material::MaterialCreateInfo mat_light_base_info = {
        .Le = {1.0, 1.0, 1.0},
        .diffuse_albedo = {0.0, 0.0, 0.0},
        .specular_albedo = {0.0, 0.0, 0.0},
        .shininess = 0.0
    };

    scene.scene_materials.reserve(8);
    mat_light_base_info.Le = {531.715, 265.857, 132.929};
    scene.scene_materials.emplace_back(mat_light_base_info);
    mat_light_base_info.Le = {50.8868, 101.774, 25.4434};
    scene.scene_materials.emplace_back(mat_light_base_info);
    mat_light_base_info.Le = {8.14188, 4.07094, 16.2838};
    scene.scene_materials.emplace_back(mat_light_base_info);
    mat_light_base_info.Le = {2.6054, 0.65135, 1.3027};
    scene.scene_materials.emplace_back(mat_light_base_info);

    scene.scene_materials.emplace_back(mat_table_info);
    mat_table_info.shininess = 1000.0;
    scene.scene_materials.emplace_back(mat_table_info);
    mat_table_info.shininess = 5000.0;
    scene.scene_materials.emplace_back(mat_table_info);
    mat_table_info.shininess = 10000.0;
    scene.scene_materials.emplace_back(mat_table_info);
    f64vec3 light_center_pos = {0, 4, -6};

    scene.scene_objects.emplace_back(Rectangle({ 
        .material = &scene.scene_materials.at(4),
        .origin = {0.0, -4.0,  2.0 },
        .normal = {0.0, 0.9935, 0.1131},
        .dimensions = {8.0, 1.0}}));

    scene.scene_objects.emplace_back(Rectangle({ 
        .material = &scene.scene_materials.at(5),
        .origin = {0.0, -3.5, -2.0},
        .normal = {0.0, 0.9496, 0.3133},
        .dimensions = {8.0, 1.0}}));

    scene.scene_objects.emplace_back(Rectangle({
        .material = &scene.scene_materials.at(6),
        .origin = {0.0, -2.5, -6.0},
        .normal = {0.0, 0.8166, 0.5751},
        .dimensions = {8.0, 1.0}}));

    scene.scene_objects.emplace_back(Rectangle({
        .material = &scene.scene_materials.at(7),
        .origin = {0.0, -1.0, -10.0},
        .normal = {0.0, 0.5400, 0.8416},
        .dimensions = {8.0, 1.0}}));

    scene.scene_objects.emplace_back(Sphere({
        .material = &scene.scene_materials.at(0), 
        .origin = light_center_pos + f64vec3{-4.5, 0.0, 0.0},
        .radius = 0.07}));

    scene.scene_objects.emplace_back(Sphere({
        .material = &scene.scene_materials.at(1),
        .origin = light_center_pos + f64vec3{-1.5, 0.0, 0.0},
        .radius = 0.16}));

    scene.scene_objects.emplace_back(Sphere({
        .material = &scene.scene_materials.at(2),
        .origin = light_center_pos + f64vec3{ 1.5, 0.0, 0.0},
        .radius = 0.4}));

    scene.scene_objects.emplace_back(Sphere({
        .material = &scene.scene_materials.at(3), 
        .origin = light_center_pos + f64vec3{ 4.5, 0.0, 0.0},
        .radius = 1.0}));

    scene.calculate_total_power();
    return scene;
}

void Application::run_loop()
{
    while(!window.get_window_should_close())
    {
        glfwPollEvents();
        std::vector<f32> img(WINDOW_DIMENSIONS.x * WINDOW_DIMENSIONS.y * 3);
        for(size_t i = 0; i < raytracer.result_image.size(); i++)
        {
            img.at(i * 3) = raytracer.result_image.at(i).R;
            img.at(i * 3 + 1) = raytracer.result_image.at(i).G;
            img.at(i * 3 + 2) = raytracer.result_image.at(i).B;
        }
        if(show_env_map)
        {
            glDrawPixels(scene.env_map.width, scene.env_map.height, GL_RGB, GL_FLOAT, scene.env_map.image.data());
            // glDrawPixels(scene.env_map.width, scene.env_map.height, GL_LUMINANCE, GL_FLOAT, scene.env_map.heat_map.data());
        } else 
        {
            glDrawPixels(WINDOW_DIMENSIONS.x, WINDOW_DIMENSIONS.y, GL_RGB, GL_FLOAT, img.data());
        }
        window.swap_buffers();
    }
}
