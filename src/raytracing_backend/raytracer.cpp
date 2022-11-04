#include "raytracer.hpp"

Raytracer::Raytracer(const u32vec2 dimensions) :
    result_image{dimensions.x * dimensions.y}, 
    working_image{dimensions.x * dimensions.y},
    sample_ratio{1.0},
    dimensions{dimensions},
    active_scene{nullptr}
{
}

static int traced_rays = 0;
void Raytracer::set_sample_ratio(f32 sample_ratio)
{
    this->sample_ratio = sample_ratio;
}

#include <iostream>
void Raytracer::trace_scene(const Scene * scene, const TraceInfo & info)
{
    active_scene = scene;
    for(u32 iteration = 1; iteration <= info.iterations; iteration++)
    {
        std::cout << "Tracing iteration num: " << iteration << std::endl;
        for(u32 y = 0; y < dimensions.y; y++)
        {
            std::cout << "progress " << u32((f32(y)/f32(dimensions.y)) * 100.0f) << "%\r" << std::flush; 
            for(u32 x = 0; x < dimensions.x; x++)
            {
                Pixel color = ray_gen(scene->camera.get_ray({x, y}, dimensions), info);
                // the same weight for all samples for computing mean incrementally
                f64 weight = 1.0 / iteration;
                Pixel new_col = color * weight + result_image.at(y * dimensions.x + x) * (1.0 - weight);
                result_image.at(y * dimensions.x + x) = color * weight + result_image.at(y * dimensions.x + x) * (1.0 - weight);
            }
        }
    }
    std::cout << "scene trace done!" << std::endl;
    std::cout << traced_rays << std::endl;
}

auto Raytracer::ray_gen(const Ray & ray, const TraceInfo & info) -> Pixel
{
    HitInfo hit = trace_ray(ray);
    if(hit.hit_distance < 0.0) { return {0.0, 0.0, 0.0}; }

    f64vec3 radiance_emitted = hit.intersectable->material->Le;
    // if albedo is low no energy will be reflected return only energy emitted by the material
    if(hit.intersectable->material->get_average_diffuse_albedo() < EPSILON &&
       hit.intersectable->material->get_average_specular_albedo() < EPSILON )
    {
        return static_cast<Pixel>(radiance_emitted);
    }
    u64 num_light_samples = info.samples * sample_ratio;
    u64 num_brdf_samples = info.samples - num_light_samples;

    // trace light sampling samples
    for(int i = 0; i < num_light_samples; i++)
    {
        const auto sample = generate_lightsource_sample(hit);
        f64vec3 outgoing_direction = sample.point - hit.hit_position;
        f64 distance_squared = dot(outgoing_direction, outgoing_direction);
        if(glm::sqrt(distance_squared) < EPSILON) { continue; }

        outgoing_direction /= glm::sqrt(distance_squared);
        f64 cos_theta_light = glm::dot(sample.normal, -outgoing_direction);
        if(cos_theta_light < EPSILON) {continue;}

        // visibility is not needed to handle, all lights are visible
        f64 pdf_light_source_sampling = sample.object->sampled_point_probability(active_scene->total_power) * distance_squared / cos_theta_light;
        f64 pdf_brdf_sampling = sample.object->material->sample_probability({hit.normal, -ray.direction, outgoing_direction});

        // the theta angle on the surface between normal and light direction
        f64 cos_theta_surface = dot(hit.normal, outgoing_direction);
        if(cos_theta_surface > 0.0)
        {
            // yes, the light is visible and contributes to the output power
            // The evaluation of rendering equation locally: (light power) * brdf * cos(theta)
            f64vec3 f = sample.object->material->Le * 
                        hit.intersectable->material->BRDF({hit.normal, -ray.direction, outgoing_direction}) *
                        cos_theta_surface;
            radiance_emitted += f / pdf_light_source_sampling / static_cast<f64>(info.samples);
        }
    }
    
    return static_cast<Pixel>(radiance_emitted);
}

auto Raytracer::generate_lightsource_sample(const HitInfo & hit) -> LightSourceSample
{
    while(true)
    {
        f64 threshold = active_scene->total_power * get_random_double();
        f64 running_power = 0.0;
        for(const auto & object : active_scene->scene_objects)
        {
            running_power += object->get_power_from_material();
            if(running_power > threshold)
            {
                const auto sample_info = object->uniformly_sample_point(hit.hit_position);
                return LightSourceSample { 
                    .object = object.get(),
                    .point = sample_info.first,
                    .normal = sample_info.second
                };
            }
        }
    }
}

auto Raytracer::trace_ray(const Ray & ray) -> HitInfo
{
    HitInfo closest_hit = HitInfo{.hit_distance = -1.0};
    for(const auto & object : active_scene->scene_objects)
    {
        traced_rays++;
        HitInfo hit = object->intersect(ray);
        if(hit.hit_distance < EPSILON) { continue; }
        if(closest_hit.hit_distance < 0.0 || hit.hit_distance < closest_hit.hit_distance)
        {
            closest_hit = hit;
        }
    }
    return closest_hit;
}