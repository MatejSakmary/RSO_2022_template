#include "utils.hpp"

static std::mt19937 engine = std::mt19937(123);
static std::uniform_real_distribution distribution = std::uniform_real_distribution<f64>(0.0, 1.0);

f64 get_random_double()
{
    // return (double)rand() / RAND_MAX;
    return distribution(engine);
}

f64vec3 get_random_double_vec()
{
    return {get_random_double(), get_random_double(), get_random_double()};
}