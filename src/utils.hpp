#pragma once

#include <random>
#include <string>
#include <stdexcept>

#include "types.hpp"
#include "tinyexr.h"
// returns random number using uniform sampling in range [0, 1)
f64 get_random_double();
f64vec3 get_random_double_vec();

auto load_exr_image(const std::string & path, std::vector<float> & img) -> void;