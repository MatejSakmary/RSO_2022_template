#pragma once

#include <vector>
#include <stdexcept>
#include <memory>

#include "operations.hpp"
#include "objects.hpp"
#include "material.hpp"
#include "camera.hpp"
#include "types.hpp"

struct Scene
{
    std::vector<Object> scene_objects;
    std::vector<Material> scene_materials;
    Camera camera;
    f64 total_power;

    Scene(const Camera & camera);
    void load_scene_from_file();
    void save_scene_to_file();
    void calculate_total_power();
};