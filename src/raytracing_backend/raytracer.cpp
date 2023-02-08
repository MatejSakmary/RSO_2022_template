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

auto Raytracer::miss_ray(const Ray & ray) -> f64vec3
{
    u32 coord = active_scene->env_map.coord_1d_from_direction(ray.direction);
    return {active_scene->env_map.image.at(coord),
            active_scene->env_map.image.at(coord + 1),
            active_scene->env_map.image.at(coord + 2)};
}

auto Raytracer::get_ray_radiance(const GetRayRadianceInfo & info) -> f64vec3
{
    f64 cos_theta_surface = glm::dot(info.prev_hit.normal, info.bounce_info.ray.direction);
    if(cos_theta_surface <= 0.0) { return {0.0, 0.0, 0.0}; }

    auto new_hit = trace_ray(info.bounce_info.ray);

    f64vec3 Le = f64vec3(0.0, 0.0, 0.0);
    f64vec3 new_hit_normal = f64vec3(0.0, 0.0, 0.0);

    if(new_hit.hit_distance == -1.0)
    {
        // ray hit nothing in the scene
        if(active_scene->use_env_map)
        {
            Le = miss_ray(info.bounce_info.ray);
            new_hit_normal = -info.bounce_info.ray.direction;
        }
        // if we are not using env map return 0
        else { return {0.0, 0.0, 0.0}; }
    } 
    else if (new_hit.hit_distance < EPSILON || new_hit.material->get_average_emmited_radiance() <= 0) {
        // ray hit either too close or the material is not emmisive
        return {0.0, 0.0, 0.0};
    } 
    else {
        // We hit an emissive object
        Le = new_hit.material->Le;
        new_hit_normal = new_hit.normal;
    }

    f64 distance_square = new_hit.hit_distance * new_hit.hit_distance;
    f64 cos_theta_light = glm::dot(new_hit_normal, -info.bounce_info.ray.direction);
    if(cos_theta_light <= EPSILON) { return {0.0, 0.0, 0.0}; }

    f64vec3 brdf_factor = info.prev_hit.material->BRDF({ info.prev_hit.normal, -info.prev_ray.direction, info.bounce_info.ray.direction});
    f64vec3 f = Le * brdf_factor * cos_theta_surface;
    
    f64 pdf_brdf_sampling = info.bounce_info.brdf_sample_prob;
    if(pdf_brdf_sampling == 0 && info.bounce_gen_method == TraceMethod::BRDF) { return {0.0, 0.0, 0.0}; }

    f64 pdf_light_sampling = info.bounce_info.light_sample_prob * distance_square / cos_theta_light;
    if(new_hit.hit_distance > EPSILON && info.method == TraceMethod::BRDF)
    {
        auto power_to_total_ratio = std::visit(GetPower{}, *new_hit.object) / active_scene->total_power;

        f64 light_sample_probability = std::visit(PointSampleProbability{new_hit.hit_position}, *new_hit.object);
        pdf_light_sampling = (power_to_total_ratio / light_sample_probability) * distance_square / cos_theta_light;
    } 

    f64 final_pdf = 0.0;
    if(info.method == TraceMethod::MULTI_IMPORTANCE_WEIGHTS)      { final_pdf = pdf_brdf_sampling + pdf_light_sampling; }
    else if (info.bounce_gen_method == TraceMethod::BRDF)         { final_pdf = pdf_brdf_sampling; }
    else if (info.bounce_gen_method == TraceMethod::LIGHT_SOURCE) { final_pdf = pdf_light_sampling; }
    // ray radiance
    return f / (final_pdf);
}

auto Raytracer::ray_gen(const Ray & ray, const TraceInfo & info) -> Pixel
{
    auto hit = trace_ray(ray);
    if(hit.hit_distance < 0.0) 
    {
        if(active_scene->use_env_map) { return Pixel(miss_ray(ray)); }
        else { return Pixel(0.0, 0.0, 0.0); }
    }

    f64vec3 radiance_emitted = hit.material->Le;
    // if albedo is low no energy will be reflected return only energy emitted by the material
    if(hit.material->get_average_diffuse_albedo() < EPSILON && hit.material->get_average_specular_albedo() < EPSILON )
    {
        return Pixel(radiance_emitted);
    }

    u64 brdf_sample_threshold = info.samples * sample_ratio;
    for(int i = 0; i < info.samples; i++)
    {
        TraceMethod bounce_method = i < brdf_sample_threshold ? TraceMethod::LIGHT_SOURCE : TraceMethod::BRDF;
        const auto bounce_info_opt = bounced_ray({.hit = hit, .incoming_ray = ray, .method = bounce_method});

        if( !bounce_info_opt.has_value()) { continue; }
        const auto bounce_info = bounce_info_opt.value();

        f64vec3 ray_radiance = get_ray_radiance({
            .bounce_info = bounce_info,
            .prev_ray = ray,
            .prev_hit = hit,
            .method = info.method,
            .bounce_gen_method = bounce_method
        });
        radiance_emitted += ray_radiance / static_cast<f64>(info.samples);
    }

    return static_cast<Pixel>(radiance_emitted);
}

auto Raytracer::bounced_ray(const GetBouncedRayInfo & info) const -> std::optional<BouncedRayInfo>
{
    auto get_new_lightsource_sample_env = [&]() -> BouncedRayInfo
    {
        f32vec3 direction = active_scene->env_map.sample_direction();
        return {
            .ray = Ray(info.hit.hit_position + 0.01 * info.hit.normal, direction),
            .light_sample_prob = active_scene->env_map.sample_probability(direction) * active_scene->env_map.width * active_scene->env_map.height,
            .brdf_sample_prob = info.hit.material->sample_probability({
                .normal = info.hit.normal,
                .view_direction = -info.incoming_ray.direction,
                .light_direction = direction
            })
        };
    };
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
        f32 light_sample_prob = 0.0f;

        if(active_scene->use_env_map)
        {
            light_sample_prob = active_scene->env_map.sample_probability(bounced_ray.direction);
        }

        return BouncedRayInfo{
            .ray = bounced_ray,
            .light_sample_prob = light_sample_prob,
            .brdf_sample_prob = brdf_probability
        };
    };

    switch(info.method)
    {
        case TraceMethod::LIGHT_SOURCE:
        {
            if(active_scene->use_env_map)
            {
                return get_new_lightsource_sample_env();
            }
            else
            {
                return get_new_lightsource_sample();
            }
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