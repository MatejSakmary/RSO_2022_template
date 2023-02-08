#pragma once

#include <random>
#include <string>
#include <stdexcept>
#include <fstream>

#include "types.hpp"
// returns random number using uniform sampling in range [0, 1)
f64 get_random_double();
f64vec3 get_random_double_vec();

auto load_hdr_image(const std::string & path, std::vector<float> & image, i32 & width, i32 & height) -> void;