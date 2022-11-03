#pragma once

#include <vector>
#include <stdexcept>
#include <memory>

#include "intersectable.hpp"
#include "material.hpp"
#include "camera.hpp"

struct Scene
{
    std::vector<std::unique_ptr<Intersectable>> scene_objects;
    std::vector<Material> scene_materials;
    Camera camera;

    Scene(const Camera & camera);
    void load_scene_from_file();
    void save_scene_to_file();
};