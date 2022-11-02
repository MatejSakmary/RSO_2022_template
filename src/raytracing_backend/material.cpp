#include "material.hpp"

Material::Material(const MaterialCreateInfo & info) : 
    Le{info.Le},
    diffuse_albedo{info.diffuse_albedo},
    specular_albedo{info.specular_albedo},
    shininess{info.shininess}
{
}

auto Material::BRDF(const MaterialEvalInfo & info) -> f64vec3
{
    f64 cos_theta_light = glm::dot(info.normal, info.light_direction);
    f64 cos_theta_view = glm::dot(info.normal, info.view_direction);

    if(cos_theta_light <= EPSILON || cos_theta_view <= EPSILON) { return {0.0, 0.0, 0.0}; }
    f64vec3 reflected = info.normal * (glm::dot(info.normal, info.light_direction) * 2.0) - info.light_direction;
    f64 cos_phi = glm::dot(info.view_direction, reflected);

    // sample is further than PI/2 from reflected direcion
    f64vec3 brdf = diffuse_albedo / M_PI;
    if(cos_phi <= 0.0) { return brdf; }

    // Max-Phong specular BRDF : symmetric and energy conserving
    return brdf + ((shininess + 1.0) / 2.0 / M_PI * pow(cos_phi, shininess) / glm::max(cos_theta_light, cos_theta_view));
}

// TODO(excercise 1)
auto Material::sample_probability(MaterialEvalInfo & info) -> f64
{
    return 0.0;
}

// TODO(excercise 1)
auto Material::sample_direction(const f64vec3 & normal, const f64vec3 & view_direction) -> std::optional<f64vec3>
{
    return {};
}