#include "intersectable.hpp"


Rectangle::Rectangle(const RectangleGeometryInfo & info, const Material* material) :
    Intersectable(material, info.origin),
    normal{glm::normalize(info.normal)},
    dimensions{info.dimensions}
{
    right = glm::normalize(glm::cross({0.0, 0.0, 1.0}, normal)); 
    forward = glm::normalize(glm::cross(normal, right));
}

IntersectInfo Rectangle::intersect(const Ray & ray)
{
    f64 denominator = glm::dot(normal, ray.direction);
    // if the ray is perpendicular to the normal it must not hit the plane
    if(glm::abs(denominator) < 1.0e-9) { return IntersectInfo{.intersection_distance = -1.0}; }

    // Intersection point must lie on the vector ray.start + hit_distance * ray.direction
    // this gives us the following formula for calculating the intersection distance
    // https://stackoverflow.com/questions/8812073/ray-and-square-rectangle-intersection-in-3d
    f64 hit_distance = glm::dot(normal, origin - ray.start) / denominator;
    if(hit_distance < 0.0) { return IntersectInfo{.intersection_distance = -1.0}; }


    f64vec3 world_hit_position = ray.start + (hit_distance * ray.direction);
    // project the hits onto the plane the rectangle lies in 
    f64 x_proj = glm::dot(world_hit_position - origin, right);
    f64 y_proj = glm::dot(world_hit_position - origin, forward); 
    // compare if the projected point is inside of the rectangle
    if(glm::abs(x_proj) > dimensions.x || glm::abs(y_proj) > dimensions.y)
    {
        return IntersectInfo{.intersection_distance = -1.0};
    }
    return IntersectInfo {
        .intersection_distance = hit_distance,
        .hit_position = world_hit_position,
        .normal = normal,
        .intersectable = this
    };
}

Sphere::Sphere(const SphereGeometryInfo & info, const Material * material) :
    Intersectable(material, info.origin),
    radius{info.radius}
{
}

IntersectInfo Sphere::intersect(const Ray & ray)
{
    f64vec3 ray_to_sphere = ray.start - origin;
    f64 a = dot(ray_to_sphere, ray.direction) * 2.0;
    f64 b = dot(ray.direction, ray.direction);
    f64 c = dot(ray_to_sphere, ray_to_sphere);
    f64 discriminant = b * b - 4.0 * a * c;

    if (discriminant < 0.0) { return IntersectInfo{.intersection_distance = -1.0}; }
    // TODO(msakmary) continue HERE
};