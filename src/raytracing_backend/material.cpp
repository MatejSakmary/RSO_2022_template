#include "material.hpp"

Material::Material(const MaterialCreateInfo & info) : 
    Le{info.Le},
    diffuse_albedo{info.diffuse_albedo},
    specular_albedo{info.specular_albedo},
    shininess{info.shininess}
{
}

auto Material::BRDF(const MaterialEvalInfo & info) const -> f64vec3
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
    return brdf + specular_albedo * ((shininess + 1.0) / 2.0 / M_PI * pow(cos_phi, shininess) / glm::max(cos_theta_light, cos_theta_view));
}

auto Material::sample_probability(const MaterialEvalInfo & info) const -> f64
{
    f64vec3 R = info.normal * (2.0f * glm::dot(info.light_direction, info.normal)) - info.light_direction;
    f64 cos_alpha = glm::dot(info.view_direction, R);
    f64 cos_theta = glm::dot(info.normal, info.light_direction);
    if(cos_theta <= 0 || cos_alpha <= 0)
    {
        return 0.0f;
    }
    f64 avg_diff_albedo = get_average_diffuse_albedo();
    f64 avg_spec_albedo = get_average_specular_albedo();
    return (avg_diff_albedo * cos_theta / M_PI) +
        (avg_spec_albedo * (shininess + 1) * glm::pow(cos_alpha, shininess) / (2.0 * M_PI));
}

auto Material::sample_direction(const f64vec3 & normal, const f64vec3 & view_direction) const -> std::optional<f64vec3>
{
    f64 e1 = get_random_double();
    f64 e2 = get_random_double();

    f64 avg_specular_albedo = get_average_specular_albedo();
    f64 avg_diffuse_albedo = get_average_diffuse_albedo();
    f64vec3 L = f64vec3(0.0, 0.0, 0.0);

    if(e1 < avg_diffuse_albedo)
    {
        f64 length = glm::sqrt(normal.x * normal.x + normal.y * normal.y);
        f64vec3 T;
        if (glm::abs(normal.x) > EPSILON && glm::abs(normal.y) > EPSILON)
        {
            T = f64vec3(normal.y / length, -normal.x / length, 0.0f);
        } else if (glm::abs(normal.y) > EPSILON)
        {
            double length = glm::sqrt(normal.y * normal.y + normal.z * normal.z);
            T = f64vec3(0.0f, -normal.z / length, normal.y / length);
        } else 
        {
            double length = glm::sqrt(normal.x * normal.x + normal.z * normal.z);
            T = f64vec3(-normal.z / length, 0.0f, normal.x / length);
        }

        f64vec3 B = glm::cross(normal, T);
        e1 = e1 / avg_diffuse_albedo;

        f64 sqrt_e1 = glm::sqrt(1.0 - e1);
        f64 x = sqrt_e1 * glm::cos(2.0 * M_PI * e2);
        f64 y = sqrt_e1 * glm::sin(2.0 * M_PI * e2);
        f64 z = glm::sqrt(e1);

        L = T * x + B * y + normal * z;

        f64 cos_theta = glm::dot(normal, L);
        if(cos_theta >= 0.0f) { return L; }
        else { return std::nullopt; }
    }
    else if(e1 < avg_diffuse_albedo + avg_specular_albedo)
    {
        f64vec3 R = normal * (2.0 * glm::dot(view_direction, normal)) - view_direction;
        f64vec3 B = glm::cross(normal, R);
        f64vec3 T = glm::cross(R, B);

        e1 = (e1 - avg_diffuse_albedo)/avg_specular_albedo;

        f64 sqrt_e1_pow = glm::sqrt(1.0f - glm::pow(e1, 2.0f / (shininess + 1.0f)));

        f64 x = sqrt_e1_pow * glm::cos(2 * M_PI * e2);
        f64 y = sqrt_e1_pow * glm::sin(2 * M_PI * e2);
        f64 z = glm::pow(e1, 1 / (shininess + 1.0f));
                 
        L = T * x + B * y + R * z;

        f64 cos_theta = glm::dot(normal, L);
        if(cos_theta >= 0.0f) { return L; }
        else { return std::nullopt; }
    }
    return std::nullopt; // error - no value
}