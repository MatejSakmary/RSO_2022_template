#include "camera.hpp"

Camera::Camera(const CameraInfo & info) :
    origin{info.origin},
    look_at{info.look_at}
{
    f64vec3 w = origin - look_at;
    f64 f = glm::length(w);
    right = glm::normalize(glm::cross(info.up, w)) * f * glm::tan(info.fov / 2.0);
    up = glm::normalize(glm::cross(w, right)) * f * glm::tan(info.fov / 2.0);
}

#include <iostream>
Ray Camera::get_ray(u32vec2 screen_coords, u32vec2 screen_dimensions) const
{
    // TODO(msakmary) Fix this to be readable
    auto right_ = right * double(double(screen_dimensions.x) / double(screen_dimensions.y));
    f64vec3 dir = look_at + 
                  right_ * (2.0 * (screen_coords.x + 0.5) / screen_dimensions.x - 1) + 
                  up * (2.0 * (screen_coords.y + 0.5) / screen_dimensions.y - 1) - origin;
    return Ray(origin, glm::normalize(dir));
}