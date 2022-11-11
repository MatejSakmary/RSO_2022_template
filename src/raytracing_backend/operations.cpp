#include "operations.hpp"
#include <cmath>

// =============================================================================================
// ======================================= INTERSECTIONS =======================================
// =============================================================================================
#pragma region intersections

auto Intersect::operator()(const Sphere & sphere) const -> HitInfo
{
    f64vec3 ray_to_sphere = ray.start - sphere.origin;
    f64 a = dot(ray.direction, ray.direction);
    f64 b = dot(ray_to_sphere, ray.direction) * 2.0;
    f64 c = dot(ray_to_sphere, ray_to_sphere) - sphere.radius * sphere.radius;
    f64 discriminant = b * b - 4.0 * a * c;

    // There is no intersection with the sphere
    if (discriminant < 0.0) { return HitInfo{.hit_distance = -1.0}; }
    f64 t1 = (-b + sqrt(discriminant)) / 2.0 / a;
    f64 t2 = (-b - sqrt(discriminant)) / 2.0 / a;
    f64 hit_distance = 0.0;
    // Both intersections are on the opposite side than the one we shot our ray 
    if (t1 <= 0.0 && t2 <= 0.0)      { return HitInfo{.hit_distance = -1.0};}
    if (t1 <= 0.0 && t2 > 0.0)       { hit_distance = t2; }
    else if (t1 > 0.0 && t2 <= 0.0)  { hit_distance = t1; }
    else if (t1 < t2)                { hit_distance = t1; }
    else                             { hit_distance = t2; }

    f64vec3 world_hit_position = ray.start + ray.direction * hit_distance;
    auto res = world_hit_position - sphere.origin;
    auto foo = glm::length(res);
    return HitInfo{
        .hit_distance = hit_distance,
        .hit_position = world_hit_position,
        .normal = (world_hit_position - sphere.origin) / sphere.radius,
        .material = sphere.material
    };
}

auto Intersect::operator()(const Rectangle & rectangle) const -> HitInfo
{
    f64 denominator = glm::dot(rectangle.normal, ray.direction);
    // if the ray is perpendicular to the normal it must not hit the plane
    if(glm::abs(denominator) < EPSILON) { return HitInfo{.hit_distance = -1.0}; }

    // Intersection point must lie on the vector ray.start + hit_distance * ray.direction
    // this gives us the following formula for calculating the intersection distance
    // https://stackoverflow.com/questions/8812073/ray-and-square-rectangle-intersection-in-3d
    f64 hit_distance = glm::dot(rectangle.normal, rectangle.origin - ray.start) / denominator;
    if(hit_distance < 0.0) { return HitInfo{.hit_distance = -1.0}; }


    f64vec3 world_hit_position = ray.start + (hit_distance * ray.direction);
    // project the hits onto the plane the rectangle lies in 
    f64 x_proj = glm::dot(world_hit_position - rectangle.origin, rectangle.right);
    f64 y_proj = glm::dot(world_hit_position - rectangle.origin, rectangle.forward); 
    // compare if the projected point is inside of the rectangle
    if(glm::abs(x_proj) > rectangle.dimensions.x || glm::abs(y_proj) > rectangle.dimensions.y)
    {
        return HitInfo{.hit_distance = -1.0};
    }
    return HitInfo {
        .hit_distance = hit_distance,
        .hit_position = world_hit_position,
        .normal = rectangle.normal,
        .material = rectangle.material
    };
}

#pragma endregion intersections

// =============================================================================================
// ======================================= UNIFORM SAMPLING ====================================
// =============================================================================================
#pragma region sample_point

auto VisiblePoint::operator()(const Sphere & sphere) const -> PointInfo
{
    f64vec3 normal = {0.0, 0.0, 0.0};
    do
    {
        normal = {get_random_double() * 2.0 - 1.0, get_random_double() * 2.0 - 1.0, get_random_double() * 2.0 - 1.0};
        if(glm::dot(view_point - sphere.origin, normal) < 0.0) { normal = -normal; }
    } while ((dot(normal, normal) > 1.0) || (glm::dot(view_point - sphere.origin, normal) < 0.0));

    normal = glm::normalize(normal);
    return {sphere.origin + normal * sphere.radius, normal};
}

auto VisiblePoint::operator()(const Rectangle & rectangle) const -> PointInfo
{
    throw std::runtime_error("[Rectangle::uniformly_sample_point()] Rectangles as light sources are not yet supported");
}

// =============================================================================================
// ======================================= POINT PROBABILITY ===================================
// =============================================================================================
#pragma region point_probability

auto PointSampleProbability::operator()(const Sphere & sphere) const -> f64
{
    return 4.0 * sphere.radius * sphere.radius * M_PI;
}

auto PointSampleProbability::operator()(const Rectangle & rectangle) const -> f64
{
    throw std::runtime_error("[Rectangle::sampled_point_probablity()] Rectangles as light sources are not yet supported");
}

#pragma endregion point_probability

// =============================================================================================
// ======================================= LIGHT POWER =========================================
// =============================================================================================
auto GetPower::operator()(const Sphere & sphere) const -> f64
{
    return sphere.material->get_average_emmited_radiance() * (4.0 * sphere.radius * sphere.radius * M_PI) * M_PI;
}

auto GetPower::operator()(const Rectangle & rectangle) const -> f64
{
    return 0.0;
}

