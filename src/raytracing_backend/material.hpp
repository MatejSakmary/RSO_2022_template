#pragma once

#include <optional>
#include <iostream>
#include "types.hpp"
#include "utils.hpp"

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
        f64vec3 Le;
        f64vec3 diffuse_albedo;
        f64vec3 specular_albedo;
        f64 shininess;
    };
    f64vec3 Le;  // the emitted power
    f64vec3 diffuse_albedo;
    f64vec3 specular_albedo;
    f64 shininess;

    Material(const MaterialCreateInfo & info);
    auto BRDF(const MaterialEvalInfo & info) const -> f64vec3;
    // TODO: excercise 1
    auto sample_probability(const MaterialEvalInfo & info) const -> f64;
    // TODO: excercise 1
    auto sample_direction(const f64vec3 & normal, const f64vec3 & view_direction) const -> std::optional<f64vec3>;

    inline auto get_average_diffuse_albedo() const -> f64
    {
        return (diffuse_albedo.r + diffuse_albedo.g + diffuse_albedo.b) / 3.0;
    }
    inline auto get_average_specular_albedo() const -> f64
    {
        return (specular_albedo.r + specular_albedo.g + specular_albedo.b) / 3.0;
    }
    inline auto get_average_emmited_radiance() const -> f64
    {
        return (Le.r + Le.g + Le.b) / 3.0; 
    }
};