#pragma once

#include "scene.hpp"
#include "types.hpp"

enum TraceMethod
{
    BRDF,
    LIGHT_SOURCE
};


struct Raytracer
{
    struct TraceInfo
    {
        u32 samples = 100;
        u32 iterations = 10;
        TraceMethod method = LIGHT_SOURCE;
    };

    struct Pixel
    {
        f64 R = 0.0;
        f64 G = 0.0;
        f64 B = 0.0;
    };

    std::vector<Pixel> result_image;

    Raytracer(const u32vec2 dimensions);

    void set_sample_ratio(f32 sample_ratio);
    void trace_scene(const Scene * scene, const TraceInfo & info);

    private:
        std::vector<Pixel> working_image;
        f32 sample_ratio;
        u32vec2 dimensions;
        // TODO(msakmary) think of a way to store active scene better
        const Scene * active_scene;

        auto trace_ray(const Ray & ray) -> Pixel;
        auto closest_hit(const Ray & ray) -> HitInfo;
};
