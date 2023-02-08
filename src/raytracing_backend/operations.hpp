#pragma once

#include "objects.hpp"
#include "types.hpp"

/// @brief Find an intersection with an object and a ray
/// @return HitInfo structure, if there was no intersection with the object and the ray
/// hit_distance in HitInfo structure is set to -1.0
struct Intersect
{
    struct HitInfo
    {
        f64 hit_distance = -1.0;
        f64vec3 hit_position = {0.0, 0.0, 0.0};
        f64vec3 normal = {0.0, 0.0, 0.0};
        const Material * material = nullptr;
        const Object * object = nullptr;
    };

    Intersect(const Ray & ray) : ray{ray} {}

    auto operator()(const Sphere & sphere) const -> HitInfo;
    auto operator()(const Rectangle & rectangle) const -> HitInfo;
    private:
        const Ray ray;
};

/// @brief get uniformly sampled point on the object which is visible from the view_point
struct VisiblePoint
{
    struct PointInfo
    {
        f64vec3 sample = {0.0, 0.0, 0.0};
        f64vec3 normal = {0.0, 0.0, 0.0};
    };
    VisiblePoint(const f64vec3 & view_point) : view_point{view_point} {}

    auto operator()(const Sphere & sphere) const -> PointInfo;
    auto operator()(const Rectangle & rectangle) const -> PointInfo;
    private:
        const f64vec3 view_point;
};

/// @brief get the probability with which the sampled point would be sampled
struct PointSampleProbability
{
    PointSampleProbability(const f64vec3 & point) : point{point} {}

    auto operator()(const Sphere & sphere) const -> f64;
    auto operator()(const Rectangle & rectangle) const -> f64;
    private:
        const f64vec3 point;
};

/// @brief get the total power corresponding to the light emmited from the object
struct GetPower
{
    auto operator()(const Sphere & sphere) const -> f64;
    auto operator()(const Rectangle & rectangle) const -> f64;
};