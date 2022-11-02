#pragma once

#include "types.hpp"
#include "material.hpp"

struct Intersectable;

struct IntersectInfo
{
    f64 intersection_distance = -1.0;
    f64vec3 hit_position = {0.0, 0.0, 0.0};
    f64vec3 normal = {0.0, 0.0, 0.0};
    const Intersectable * intersectable = nullptr;
};

struct Intersectable
{
    const Material* material;
    f64vec3 origin;

    virtual auto intersect(const Ray & ray) -> IntersectInfo = 0;

    protected:
        Intersectable(const Material * material, const f64vec3 & origin) : 
            material{material}, origin{origin} {}
};

struct Rectangle : public Intersectable
{
    struct RectangleGeometryInfo
    {
        const f64vec3 & origin = {0.0, 0.0, 0.0};
        const f64vec3 & normal = {0.0, 1.0, 0.0};
        const f64vec2 & dimensions = {1.0, 1.0};
    };
    f64vec3 normal;
    f64vec3 right;
    f64vec3 forward;
    f64vec2 dimensions;

    Rectangle(const RectangleGeometryInfo & info, const Material* material);
    auto intersect(const Ray & ray) -> IntersectInfo override;
};

struct Sphere : public Intersectable
{
    struct SphereGeometryInfo
    {
        const f64vec3 & origin = {0.0, 0.0, 0.0};
        const f64 radius = 1.0;
    };
    f64 radius;

    Sphere(const SphereGeometryInfo & info, const Material* material);
    auto intersect(const Ray & ray) -> IntersectInfo override;
};