#pragma once

#include "types.hpp"

struct Camera
{
    struct CameraInfo
    {
        const f64vec3 origin;
        const f64vec3 look_at;
        const f64vec3 up;
        const f64 fov;
    };

    f64vec3 origin;
    f64vec3 look_at;
    f64vec3 right;
    f64vec3 up;

    Camera(const CameraInfo & info);
    Ray get_ray(u32vec2 screen_coords, u32vec2 screen_dimensions) const;
    private:
};