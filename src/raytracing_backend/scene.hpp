#pragma once

#include <vector>
#include <stdexcept>
#include <memory>
#include <span>

#include "operations.hpp"
#include "objects.hpp"
#include "material.hpp"
#include "camera.hpp"
#include "types.hpp"

struct SampleRet
{
    f64 sample;
    f64 probability;
};

struct EnvironmentMap
{
    struct ProbabilityColumn
    {
        std::vector<i32> samples_cnt;
        std::vector<f64> CDF;
        std::vector<f64> probability;
        f64 column_sum;

        void init(const std::span<f64> intensities, f64 total_col_intensity);
        [[nodiscard]] auto sample(f64 u) -> SampleRet;
    };

    i32 height;
    i32 width;
    float total_power;
    std::vector<f32> image;
    std::vector<f64> lum_image;
    std::vector<f64> row_prob;
    std::vector<ProbabilityColumn> columns;
    ProbabilityColumn top_level;

    void init();
    [[nodiscard]] auto sample_direction() -> f64vec3;
    [[nodiscard]] auto sample_probability(const f64vec3 direction) -> f64;
    [[nodiscard]] auto coords_2d_from_direction(const f64vec3 direction) -> std::pair<u32vec2, f64>;
    [[nodiscard]] auto coord_1d_from_direction(const f64vec3 direction) -> u32;
};

struct Scene
{
    std::vector<Object> scene_objects;
    std::vector<Material> scene_materials;

    EnvironmentMap env_map;
    bool use_env_map;
    Camera camera;
    f64 total_power;

    Scene(const Camera & camera);
    void load_scene_from_file();
    void save_scene_to_file();
    void calculate_total_power();
    
};