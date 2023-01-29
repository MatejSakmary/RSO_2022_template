#include "raytracer.hpp"
#include <omp.h>
#include <glm/gtx/rotate_vector.hpp>
#include <glm/gtx/compatibility.hpp>
#include <cmath>

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

void Raytracer::trace_scene(Scene * scene, const TraceInfo & info)
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


auto Raytracer::miss_ray(const Ray & ray) -> Pixel
{
    // const f64vec3 rotated_direction = glm::rotateX(ray.direction, f64(glm::radians(180.0f)));
    f64 theta = acos(ray.direction.z);
    f64 phi = atan2(ray.direction.y, ray.direction.x);

    if(phi < 0) { phi += 2.0f * M_PI; }

    i32 u = phi/ (2 * M_PI) * u32(active_scene->env_map.width);
    glm::clamp(u, 0, i32(active_scene->env_map.width - 1));

    i32 v = theta/M_PI * u32(active_scene->env_map.height);
    glm::clamp(v, 0, i32(active_scene->env_map.height - 1));

    u32 coord = ( v * active_scene->env_map.width + u) * 3;

    return {active_scene->env_map.image.at(coord),
            active_scene->env_map.image.at(coord + 1),
            active_scene->env_map.image.at(coord + 2)};
}

auto Raytracer::ray_gen(const Ray & ray, const TraceInfo & info) -> Pixel
{
    auto hit = trace_ray(ray);
    if(hit.hit_distance < 0.0) { 
        return miss_ray(ray); 
    }

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
        const auto outgoing_info_opt = bounced_ray({
            .hit = hit,
            .incoming_ray = ray,
            .method = TraceMethod::LIGHT_SOURCE
        });

        if(!outgoing_info_opt.has_value()) {continue;}
        const auto outgoing_info = outgoing_info_opt.value();

        auto new_hit = trace_ray(outgoing_info.ray);
        if(new_hit.hit_distance < EPSILON) { continue; }

        f64 cos_theta_light = glm::dot(new_hit.normal, -outgoing_info.ray.direction);
        if(cos_theta_light < EPSILON || new_hit.material->get_average_emmited_radiance() <= 0) {continue;}

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
            if(info.method == TraceMethod::MULTI_IMPORTANCE_WEIGHTS)
            {
                radiance_emitted += f / (pdf_brdf_sampling + pdf_light_source_sampling) / static_cast<f64>(info.samples);
            } else {
                radiance_emitted += f / pdf_light_source_sampling / static_cast<f64>(info.samples);
            }
        }
    }

    // trace brdf sampling samples
    for(int i = 0; i < num_brdf_samples; i++)
    {
        const auto outgoing_info_opt = bounced_ray({
            .hit = hit,
            .incoming_ray = ray,
            .method = TraceMethod::BRDF
        });

        if(!outgoing_info_opt.has_value()) { continue;}
        const auto outgoing_info = outgoing_info_opt.value();

        f64 cos_theta_surface = glm::dot(hit.normal, outgoing_info.ray.direction);

        if(cos_theta_surface <= 0) { continue;}

        auto new_hit = trace_ray(outgoing_info.ray);
        if(new_hit.hit_distance == -1.0 && outgoing_info.brdf_sample_prob > 0.0f) 
        {
            if(!active_scene->use_env_map)
            {
                continue;
            }
            auto tmp = miss_ray(outgoing_info.ray);
            new_hit.hit_position.x = tmp.R;
            new_hit.hit_position.y = tmp.G;
            new_hit.hit_position.z = tmp.B;
            new_hit.normal = -outgoing_info.ray.direction;
        } else if(new_hit.hit_distance < EPSILON || new_hit.material->get_average_emmited_radiance() <= 0) 
        {
            continue;
        }

        f64 distance_square = new_hit.hit_distance * new_hit.hit_distance;
        f64 cos_theta_light = glm::dot(new_hit.normal, -outgoing_info.ray.direction);

        if(cos_theta_light <= EPSILON) { continue; }

        f64 pdf_brdf_sampling = outgoing_info.brdf_sample_prob;
        f64vec3 f;

        f64 pdf_light_source_sampling;

        if(new_hit.hit_distance < EPSILON)
        {
            f = new_hit.hit_position * 
                hit.material->BRDF({ hit.normal, -ray.direction, outgoing_info.ray.direction }) *
                cos_theta_surface;
        } else {
            const auto power_to_total_ratio = std::visit(GetPower{}, *new_hit.object) / active_scene->total_power;
            const f64 light_sample_prob = power_to_total_ratio / std::visit(PointSampleProbability{new_hit.hit_position}, *new_hit.object);
            pdf_light_source_sampling = light_sample_prob * distance_square / cos_theta_light;
            f = new_hit.material->Le * 
                hit.material->BRDF({ hit.normal, -ray.direction, outgoing_info.ray.direction }) *
                cos_theta_surface;
        }
        if(info.method == TraceMethod::MULTI_IMPORTANCE_WEIGHTS)
        {
            radiance_emitted += f / (pdf_brdf_sampling + pdf_light_source_sampling) / static_cast<f64>(info.samples);
        } else {
            radiance_emitted += f / pdf_brdf_sampling / static_cast<f64>(info.samples);
        }
    }
    
    return static_cast<Pixel>(radiance_emitted);
}

auto Raytracer::bounced_ray(const GetBouncedRayInfo & info) const -> std::optional<BouncedRayInfo>
{
    auto get_new_lightsource_sample = [&]() -> BouncedRayInfo
    {
        while(true)
        {
            f64 threshold = active_scene->total_power * get_random_double();
            f64 running_power = 0.0;
            if(active_scene->use_env_map)
            {
                return {};
            } 
            else 
            {
                for(const auto & object : active_scene->scene_objects)
                {
                    running_power += std::visit(GetPower{}, object);
                    if(running_power > threshold)
                    {
                        const auto light_sample = std::visit(VisiblePoint{info.hit.hit_position}, object );
                        const auto power_to_total_ratio = std::visit(GetPower{}, object) / active_scene->total_power;
                        const Ray bounced_ray = Ray(info.hit.hit_position + 0.01 * info.hit.normal, light_sample.sample - info.hit.hit_position);

                        const f64 brdf_probability = info.hit.material->sample_probability({
                            .normal = info.hit.normal,
                            .view_direction = -info.incoming_ray.direction,
                            .light_direction = bounced_ray.direction
                        });

                        return BouncedRayInfo{
                            .ray = bounced_ray,
                            .light_sample_prob = power_to_total_ratio / std::visit(PointSampleProbability{light_sample.sample}, object),
                            .brdf_sample_prob = brdf_probability
                        };
                    }
                }
            }
        }
    };

    auto get_new_brdf_sample = [&]() -> std::optional<BouncedRayInfo>
    {
        auto ray_dir = info.hit.material->sample_direction(info.hit.normal, -info.incoming_ray.direction);
        if(!ray_dir.has_value()) { return std::nullopt; }

        const Ray bounced_ray = Ray(info.hit.hit_position, ray_dir.value());
        const f64 brdf_probability = info.hit.material->sample_probability({
            .normal = info.hit.normal,
            .view_direction = -info.incoming_ray.direction,
            .light_direction = bounced_ray.direction
        });

        return BouncedRayInfo{
            .ray = bounced_ray,
            .light_sample_prob = 0.0f, // Unable to determine at this point
            .brdf_sample_prob = brdf_probability
        };
    };

    switch(info.method)
    {
        case TraceMethod::LIGHT_SOURCE:
        {
            return get_new_lightsource_sample();
        }
        case TraceMethod::BRDF:
        {
            return get_new_brdf_sample();
        }
        default:
        {
            throw std::runtime_error("[Raytracer::bounced_ray()] ERROR Unknown sampling method");
            return std::nullopt;
        }
    }
}

auto Raytracer::trace_ray(const Ray & ray) -> Intersect::HitInfo
{
    Intersect::HitInfo closest_hit {};
    for(const auto & object : active_scene->scene_objects)
    {
        Intersect::HitInfo hit = std::visit(Intersect{ray}, object);
        if(active_scene->use_env_map == true && std::holds_alternative<Sphere>(object)) { continue; }

        if(hit.hit_distance < EPSILON) { continue; }
        if(closest_hit.hit_distance < 0.0 || hit.hit_distance < closest_hit.hit_distance)
        {
            closest_hit = hit;
            closest_hit.object = &object;
        }
    }
    return closest_hit;
}