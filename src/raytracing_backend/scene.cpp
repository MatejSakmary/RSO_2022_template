#include "scene.hpp"

Scene::Scene(const Camera & camera) : camera{camera}
{
}

void Scene::load_scene_from_file()
{
    throw std::runtime_error("[Scene::load_scene_from_file()] not yet implemented");
}
void Scene::save_scene_to_file()
{
    throw std::runtime_error("[Scene::save_scene_to_file()] not yet implemented");
}