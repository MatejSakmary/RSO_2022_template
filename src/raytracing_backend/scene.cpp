#include "scene.hpp"

#include <iostream>

Scene::Scene(const Camera & camera) : camera{camera}, total_power{0.0}
{
}
void Scene::calculate_total_power()
{
    total_power = 0.0;
    for(const auto & object : scene_objects)
    {
        total_power += std::visit(GetPower{}, object);
    }
    std::cout << "total scene power : " << total_power << std::endl;
}

void Scene::load_scene_from_file()
{
    throw std::runtime_error("[Scene::load_scene_from_file()] not yet implemented");
}
void Scene::save_scene_to_file()
{
    throw std::runtime_error("[Scene::save_scene_to_file()] not yet implemented");
}