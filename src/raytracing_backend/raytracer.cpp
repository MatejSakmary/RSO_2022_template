#include "raytracer.hpp"

Raytracer::Raytracer(const u32vec2 dimensions) :
    result_image{dimensions.x * dimensions.y}, 
    working_image{dimensions.x * dimensions.y},
    sample_ratio{0.0},
    dimensions{dimensions},
    active_scene{nullptr}
{
}

void Raytracer::set_sample_ratio(f32 sample_ratio)
{
    this->sample_ratio = sample_ratio;
}

void Raytracer::trace_scene(const Scene * scene, const TraceInfo & info)
{
    active_scene = scene;
    for(u32 iteration = 0; iteration < info.iterations; iteration++)
    {
        for(u32 y = 0; y < dimensions.y; y++)
        {
            for(u32 x = 0; x < dimensions.x; x++)
            {
                Pixel color = trace_ray(scene->camera.get_ray({x, y}, dimensions));
                result_image.at(y * dimensions.x + x) = color;
            }
        }
    }
}

auto Raytracer::trace_ray(const Ray & ray) -> Pixel
{
    HitInfo hit = closest_hit(ray);
    if(hit.hit_distance > 0.0) { return {1.0, 0.0, 0.0}; }
    return {0.0, 0.0, 0.0};
};

auto Raytracer::closest_hit(const Ray & ray) -> HitInfo
{
    HitInfo closest_hit = HitInfo{.hit_distance = -1.0};
    for(const auto & object : active_scene->scene_objects)
    {
        HitInfo hit = object->intersect(ray);
        if(hit.hit_distance < EPSILON) { continue; } 
        if(closest_hit.hit_distance < 0.0 || hit.hit_distance < closest_hit.hit_distance)
        {
            closest_hit = hit;
        }
    }
    return closest_hit;
}