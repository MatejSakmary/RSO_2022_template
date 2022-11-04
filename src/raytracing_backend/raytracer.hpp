#pragma once

#include "scene.hpp"
#include "types.hpp"

enum TraceMethod
{
    BRDF,
    LIGHT_SOURCE
};

struct LightSourceSample
{
    Intersectable * object;
    f64vec3 point;
    f64vec3 normal;
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

        Pixel() = default;
        Pixel(const f64 & R, const f64 & G, const f64 & B) : R{R}, G{G}, B{B} {}
        explicit Pixel(const f64vec3 & color) : R{color.r}, G{color.g}, B{color.b} {}
        Pixel operator *(const f64 val) { f64vec3 tmp{R, G, B}; return static_cast<Pixel>(tmp * val); }
        Pixel operator +(const Pixel & other) { return { R + other.R, G + other.G, B + other.B }; }
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

        auto ray_gen(const Ray & ray, const TraceInfo & info) -> Pixel;
        auto trace_ray(const Ray & ray) -> HitInfo;
        auto generate_lightsource_sample(const HitInfo & hit) -> LightSourceSample;
};
