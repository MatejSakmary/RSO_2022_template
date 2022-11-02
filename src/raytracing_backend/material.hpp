#pragma once

#include <optional>
#include "types.hpp"

struct Material
{
    struct MaterialEvalInfo
    {
        const f64vec3 normal;
        // outgoing view direction
        const f64vec3 view_direction;  
        // incoming light direction
        const f64vec3 light_direction; 
    };

    struct MaterialCreateInfo
    {
        const f64vec3 Le;
        const f64vec3 diffuse_albedo;
        const f64vec3 specular_albedo;
        const f64 shininess;
    };
    f64vec3 Le;  // the emitted power
    f64vec3 diffuse_albedo;
    f64vec3 specular_albedo;
    f64 shininess;

    Material(const MaterialCreateInfo & info);
    auto BRDF(const MaterialEvalInfo & info) -> f64vec3;
    auto sample_probability(MaterialEvalInfo & info) -> f64;
    auto sample_direction(const f64vec3 & normal, const f64vec3 & view_direction) -> std::optional<f64vec3>;
};