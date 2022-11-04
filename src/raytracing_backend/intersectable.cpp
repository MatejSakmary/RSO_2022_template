#include "intersectable.hpp"

////////////////////////////////////////////////////////////////////////////
//                                RECTANGLE                               //
////////////////////////////////////////////////////////////////////////////
Rectangle::Rectangle(const RectangleGeometryInfo & info, const Material* material) :
    Intersectable(material, info.origin),
    normal{glm::normalize(info.normal)},
    dimensions{info.dimensions}
{
    right = glm::normalize(glm::cross({0.0, 0.0, 1.0}, normal)); 
    forward = glm::normalize(glm::cross(normal, right));
}

auto Rectangle::intersect(const Ray & ray) const -> HitInfo
{
    f64 denominator = glm::dot(normal, ray.direction);
    // if the ray is perpendicular to the normal it must not hit the plane
    if(glm::abs(denominator) < EPSILON) { return HitInfo{.hit_distance = -1.0}; }

    // Intersection point must lie on the vector ray.start + hit_distance * ray.direction
    // this gives us the following formula for calculating the intersection distance
    // https://stackoverflow.com/questions/8812073/ray-and-square-rectangle-intersection-in-3d
    f64 hit_distance = glm::dot(normal, origin - ray.start) / denominator;
    if(hit_distance < 0.0) { return HitInfo{.hit_distance = -1.0}; }


    f64vec3 world_hit_position = ray.start + (hit_distance * ray.direction);
    // project the hits onto the plane the rectangle lies in 
    f64 x_proj = glm::dot(world_hit_position - origin, right);
    f64 y_proj = glm::dot(world_hit_position - origin, forward); 
    // compare if the projected point is inside of the rectangle
    if(glm::abs(x_proj) > dimensions.x || glm::abs(y_proj) > dimensions.y)
    {
        return HitInfo{.hit_distance = -1.0};
    }
    return HitInfo {
        .hit_distance = hit_distance,
        .hit_position = world_hit_position,
        .normal = normal,
        .intersectable = this
    };
}

auto Rectangle::uniformly_sample_point(const f64vec3 & illuminated_point) const -> std::pair<f64vec3, f64vec3>
{
    throw std::runtime_error("[Rectangle::uniformly_sample_point()] Rectangles as light sources are not yet supported");
}

auto Rectangle::sampled_point_probability(f64 total_power) const -> f64
{
    throw std::runtime_error("[Rectangle::sampled_point_probablity()] Rectangles as light sources are not yet supported");
}

auto Rectangle::get_power_from_material() const -> f64
{
    return 0.0;
}
////////////////////////////////////////////////////////////////////////////
//                                SPHERE                                  //
////////////////////////////////////////////////////////////////////////////
Sphere::Sphere(const SphereGeometryInfo & info, const Material * material) :
    Intersectable(material, info.origin),
    radius{info.radius}
{
}

#include <iostream>
auto Sphere::intersect(const Ray & ray) const -> HitInfo
{
    f64vec3 ray_to_sphere = ray.start - origin;
    f64 a = dot(ray.direction, ray.direction);
    f64 b = dot(ray_to_sphere, ray.direction) * 2.0;
    f64 c = dot(ray_to_sphere, ray_to_sphere) - radius * radius;
    f64 discriminant = b * b - 4.0 * a * c;

    // There is no intersection with the sphere
    if (discriminant < 0.0) { return HitInfo{.hit_distance = -1.0}; }
    f64 t1 = (-b + glm::sqrt(discriminant)) / 2.0 / a;
    f64 t2 = (-b - glm::sqrt(discriminant)) / 2.0 / a;
    f64 hit_distance = 0.0;
    // Both intersections are on the opposite side than the one we shot our ray 
    if (t1 <= 0.0 && t2 <= 0.0)      { return HitInfo{.hit_distance = -1.0};}
    if (t1 <= 0.0 && t2 > 0.0)       { hit_distance = t2; }
    else if (t1 > 0.0 && t2 <= 0.0)  { hit_distance = t1; }
    else if (t1 < t2)                { hit_distance = t1; }
    else                             { hit_distance = t2; }

    f64vec3 world_hit_position = ray.start + ray.direction * hit_distance;
    return HitInfo{
        .hit_distance = hit_distance,
        .hit_position = world_hit_position,
        .normal = (world_hit_position - origin) / radius,
        .intersectable = this
    };
};

auto Sphere::uniformly_sample_point(const f64vec3 & illuminated_point) const -> std::pair<f64vec3, f64vec3>
{
    f64vec3 normal = {0.0, 0.0, 0.0};
    do
    {
        normal = {get_random_double() * 2.0 - 1.0, get_random_double() * 2.0 - 1.0, get_random_double() * 2.0 - 1.0};
        if(glm::dot(illuminated_point - origin, normal) < 0.0) { continue; }
    } while (dot(normal, normal) > 1.0);

    normal = glm::normalize(normal);
    return {origin + normal * radius, normal};
}

auto Sphere::sampled_point_probability(f64 total_power) const -> f64
{
    return get_power_from_material() / total_power / (4.0 * radius * radius * M_PI);
}

auto Sphere::get_power_from_material() const -> f64
{
    return material->get_average_emmited_radiance() * (4.0 * radius * radius * M_PI) * M_PI;
}