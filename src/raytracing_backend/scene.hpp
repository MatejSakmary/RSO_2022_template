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
    f32 sample;
    f32 probability;
};

struct EnvironmentMap
{
    struct ProbabilityColumn
    {
        std::vector<f32> CDF;
        std::vector<f32> probability;
        f32 column_sum;

        void init(const std::span<f32> intensities, f32 total_col_intensity);
        [[nodiscard]] auto sample(f32 u) const -> SampleRet;
    };

    i32 height;
    i32 width;
    float total_power;
    std::vector<f32> image;
    std::vector<f32> lum_image;
    std::vector<f32> row_prob;
    std::vector<ProbabilityColumn> columns;
    ProbabilityColumn top_level;

    void init();
    [[nodiscard]] auto sample_direction() const -> f32vec3;
    [[nodiscard]] auto sample_probability(const f32vec3 direction) const -> f32;
    [[nodiscard]] auto coords_2d_from_direction(const f32vec3 direction) const -> std::pair<u32vec2, f64>;
    [[nodiscard]] auto coord_1d_from_direction(const f32vec3 direction) const -> u32;
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