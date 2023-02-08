#pragma once

#include <stdexcept>
#include <utility>

#include "types.hpp"
#include "material.hpp"
#include "utils.hpp"


struct Rectangle
{
    struct RectangleGeometryInfo
    { 
        const Material* material = nullptr;
        const f64vec3 & origin = {0.0, 0.0, 0.0};
        const f64vec3 & normal = {0.0, 1.0, 0.0};
        const f64vec2 & dimensions = {1.0, 1.0};
    };

    const Material* material;

    f64vec3 origin;
    f64vec3 normal;
    f64vec3 right;
    f64vec3 forward;
    f64vec2 dimensions;

    Rectangle(const RectangleGeometryInfo & info) :
        material{info.material},
        origin{info.origin},
        normal{glm::normalize(info.normal)},
        dimensions{info.dimensions}
    {
        right = glm::normalize(glm::cross({0.0, 0.0, 1.0}, normal)); 
        forward = glm::normalize(glm::cross(normal, right));
    }
};

struct Sphere
{
    struct SphereGeometryInfo
    {
        const Material* material = nullptr;
        const f64vec3 & origin = {0.0, 0.0, 0.0};
        const f64 radius = 1.0;
    };

    const Material* material;

    f64vec3 origin;
    f64 radius;

    Sphere(const SphereGeometryInfo & info) :
        material{info.material},
        origin{info.origin},
        radius{info.radius}
    {}
};