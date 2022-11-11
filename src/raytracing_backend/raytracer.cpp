#include "raytracer.hpp"
#include <omp.h>

Raytracer::Raytracer(const u32vec2 dimensions) :
    result_image{dimensions.x * dimensions.y}, 
    working_image{dimensions.x * dimensions.y},
    sample_ratio{1.0},
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
    omp_set_num_threads(omp_get_num_procs() * 2);
    active_scene = scene;
    for(u32 iteration = 1; iteration <= info.iterations; iteration++)
    {
        std::cout << "Tracing iteration num: " << iteration << std::endl;
        for(u32 y = 0; y < dimensions.y; y++)
        {
            std::cout << "progress " << u32((f32(y)/f32(dimensions.y)) * 100.0f) << "%\r" << std::flush; 
            #pragma omp parallel for schedule(dynamic) 
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
}

auto Raytracer::ray_gen(const Ray & ray, const TraceInfo & info) -> Pixel
{
    auto hit = trace_ray(ray);
    if(hit.hit_distance < 0.0) { return {0.0, 0.0, 0.0}; }

    f64vec3 radiance_emitted = hit.material->Le;
    // if albedo is low no energy will be reflected return only energy emitted by the material
    if(hit.material->get_average_diffuse_albedo() < EPSILON &&
       hit.material->get_average_specular_albedo() < EPSILON )
    {
        return static_cast<Pixel>(radiance_emitted);
    }
    u64 num_light_samples = info.samples * sample_ratio;
    u64 num_brdf_samples = info.samples - num_light_samples;

    // trace light sampling samples
    for(int i = 0; i < num_light_samples; i++)
    {
        const auto outgoing_info = bounced_ray(hit, TraceMethod::LIGHT_SOURCE);
        auto new_hit = trace_ray(outgoing_info.ray);
        if(new_hit.hit_distance < EPSILON) { continue; }

        f64 cos_theta_light = glm::dot(new_hit.normal, -outgoing_info.ray.direction);
        if(cos_theta_light < EPSILON || new_hit.material->get_average_emmited_radiance() < EPSILON) {continue;}

        f64 pdf_light_source_sampling = outgoing_info.light_sample_prob * (new_hit.hit_distance * new_hit.hit_distance) / cos_theta_light;
        f64 pdf_brdf_sampling = outgoing_info.brdf_sample_prob;
        // the theta angle on the surface between normal and light direction
        f64 cos_theta_surface = dot(hit.normal, outgoing_info.ray.direction);
        if(cos_theta_surface > 0.0)
        {
            // yes, the light is visible and contributes to the output power
            // The evaluation of rendering equation locally: (light power) * brdf * cos(theta)
            f64vec3 f = new_hit.material->Le * 
                        hit.material->BRDF({hit.normal, -ray.direction, outgoing_info.ray.direction}) *
                        cos_theta_surface;
            radiance_emitted += f / pdf_light_source_sampling / static_cast<f64>(info.samples);
        }
    }
    
    return static_cast<Pixel>(radiance_emitted);
}

auto Raytracer::bounced_ray(const Intersect::HitInfo & hit, TraceMethod method) -> BouncedRayInfo
{
    while(true)
    {
        f64 threshold = active_scene->total_power * get_random_double();
        f64 running_power = 0.0;
        auto prnt_vec = [](const f64vec3 & vec) { std::cout << vec.x << " " << vec.y << " " << vec.z << std::endl; };
        for(const auto & object : active_scene->scene_objects)
        {
            running_power += std::visit(GetPower{}, object);
            if(running_power > threshold)
            {
                const auto light_sample = std::visit(VisiblePoint{hit.hit_position}, object );
                const auto power_to_total_ratio = std::visit(GetPower{}, object) / active_scene->total_power;

                return BouncedRayInfo{
                    .ray = { hit.hit_position, glm::normalize(light_sample.sample - hit.hit_position)},
                    .light_sample_prob = power_to_total_ratio / std::visit(PointSampleProbability{light_sample.sample}, object),
                    .brdf_sample_prob = 0.0
                };
            }
        }
    }
}

auto Raytracer::trace_ray(const Ray & ray) -> Intersect::HitInfo
{
    Intersect::HitInfo closest_hit {};
    for(const auto & object : active_scene->scene_objects)
    {
        Intersect::HitInfo hit = std::visit(Intersect{ray}, object);
        if(hit.hit_distance < EPSILON) { continue; }
        if(closest_hit.hit_distance < 0.0 || hit.hit_distance < closest_hit.hit_distance)
        {
            closest_hit = hit;
        }
    }
    return closest_hit;
}