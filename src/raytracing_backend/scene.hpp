#pragma once

#include <vector>
#include <stdexcept>
#include <memory>

#include "operations.hpp"
#include "objects.hpp"
#include "material.hpp"
#include "camera.hpp"
#include "types.hpp"

struct EnvironmentMap
{
    float height;
    float width;
    float total_power;
    std::vector<float> image;
    std::vector<float> lum_image;
    std::vector<float> col_prob;
    std::vector<float> heat_map;

    void init();
};

struct Scene
{
    std::vector<Object> scene_objects;
    std::vector<Material> scene_materials;

    EnvironmentMap env_map;
    bool use_env_map;
    Camera camera;
    f64 total_power;

    Scene(const Camera & camera);
    void load_scene_from_file();
    void save_scene_to_file();
    void calculate_total_power();
    
};