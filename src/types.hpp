#pragma once
#include <cstdint>
#include <variant>
#include <glm/glm.hpp>

#define _USE_MATH_DEFINES
#include <math.h>

using u8 = std::uint8_t;
using u16 = std::uint16_t;
using u32 = std::uint32_t;
using u64 = std::uint64_t;
using usize = std::size_t;
using b32 = u32;

using i8 = std::int8_t;
using i16 = std::int16_t;
using i32 = std::int32_t;
using i64 = std::int64_t;
using isize = std::ptrdiff_t;

using f32 = float;
using f64 = double;

using f32vec2 = glm::vec2;
using f32vec3 = glm::vec3;
using f32vec4 = glm::vec4;

using f64vec2 = glm::dvec2;
using f64vec3 = glm::dvec3;
using f64vec4 = glm::dvec4;

using u32vec2 = glm::uvec2;
using u32vec3 = glm::uvec3;
using u32vec4 = glm::uvec4;

struct Ray
{
    f64vec3 start = {0.0, 0.0, 0.0};
    f64vec3 direction = {0.0, 0.0, 0.0};

    Ray(const f64vec3 & start, const f64vec3 & direction) :
        start{start}, direction{glm::normalize(direction)} 
    {
    }
};

const f64 EPSILON = 1.0e-9;

// forward declare all object types
struct Sphere;
struct Rectangle;
using Object = std::variant<Sphere, Rectangle>;