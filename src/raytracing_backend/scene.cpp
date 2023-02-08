#include "scene.hpp"

#include <iostream>

void EnvironmentMap::ProbabilityColumn::init(const std::span<f64> column, f64 total_col_intensity) 
{
    samples_cnt = std::vector<i32>(column.size(),0);
    CDF = std::vector<f64>(column.size() + 1);
    probability.reserve(column.size());
    std::copy(column.begin(), column.end(), std::back_inserter(probability));

    for(int i = 0; i < column.size(); i++)
    {
        CDF.at(i + 1) = (CDF.at(i) + column[i]);
    }
    column_sum = total_col_intensity;
}

auto EnvironmentMap::ProbabilityColumn::sample(f64 u) -> SampleRet
{
    auto sample = std::lower_bound(CDF.begin(), CDF.end(), glm::clamp(u * column_sum, 0.0, CDF.at(CDF.size() - 1))); 
    i32 offset = sample == CDF.end() ? CDF.size() - 3 : i32(std::distance(CDF.begin(), sample) - 1);
    samples_cnt.at(offset) += 1;
    f64 g = (u - CDF.at(offset) / (CDF.at(offset + 1) - CDF.at(offset)));
    g = glm::clamp(g, 0.0, 1.0);

    return {
        .sample = f64(offset) + g,
        .probability = probability.at(offset) / column_sum
    };
}

void EnvironmentMap::init()
{
    lum_image = std::vector<f64>(width * height);
    columns = std::vector<ProbabilityColumn>(width);
    std::vector<f64> top_level_intensities(width);
    total_power = 0.0f;

    for(size_t x = 0; x < width; x++)
    {
        f64 total_col_intensity = 0;
        f64 CDF = 0;
        for(size_t y = 0; y < height; y++)
        {
            // y = 0        -> angle = 0
            // y = height/2 -> angle = pi/2
            // y = height   -> angle = pi
            f64 uv = (f64(y) + 0.5f) / f64(height);
            // spherical env map is set up in such a way that the first row is the top
            // of sphere. This than has the same area as the row in the middle of the sphere
            // thus we need to normalize by sin factor
            f64 norm_factor = sin(M_PI * uv);

            i32 luminance_idx = x * height + y;
            i32 color_index = (y * width + x) * 3;

            lum_image.at(luminance_idx) = ( 
                0.2126f * image.at(color_index) +
                0.7152f * image.at(color_index + 1) +
                0.0722f * image.at(color_index + 2)) * norm_factor;
            total_power += lum_image.at(luminance_idx);
            total_col_intensity += lum_image.at(luminance_idx);
        }
        top_level_intensities.at(x) = total_col_intensity; 
        columns.at(x).init(std::span<f64>(lum_image.begin() + x * height, height), total_col_intensity);
    }
    top_level.init(std::span<f64>(top_level_intensities.begin(), top_level_intensities.size()), total_power);

    // image = std::vector<f32>(width * height * 3);
    // for(int i = 0; i < 100000; i++)
    // {
    //     auto dir = sample_direction();
    //     auto coord = coord_1d_from_direction(dir);
    //     image.at(coord) = 1.0f;
    // }
    std::cout << "Total power: " << total_power << std::endl;
}

auto EnvironmentMap::coord_1d_from_direction(const f64vec3 direction) -> u32
{
    f64 theta = acos(direction.z);
    f64 phi = atan2(direction.y, direction.x);

    if(phi < 0) { phi += 2.0f * M_PI; }

    i32 u = phi/ (2 * M_PI) * u32(width);
    u = glm::clamp(u, 0, i32(width - 1));

    i32 v = theta/M_PI * u32(height);
    v = glm::clamp(v, 0, i32(height - 1));

    u32 coord = ( v * u32(width) + u) * 3;

    return coord;
}

auto EnvironmentMap::coords_2d_from_direction(const f64vec3 direction) -> std::pair<u32vec2, f64>
{
    f64 theta = acos(direction.z);
    f64 phi = atan2(direction.y, direction.x);

    if(phi < 0) { phi += 2.0f * M_PI; }

    i32 u = phi/ (2 * M_PI) * u32(width);
    u = glm::clamp(u, 0, i32(width - 1));

    i32 v = theta/M_PI * u32(height);
    v = glm::clamp(v, 0, i32(height - 1));

    return {u32vec2(u, v), theta};
}

auto EnvironmentMap::sample_probability(const f64vec3 direction) -> f64
{
    auto res = coords_2d_from_direction(direction);
    u32vec2 uv = res.first;
    f64 theta = res.second;

    f64 pdf = (top_level.probability.at(uv.x) * columns.at(uv.x).probability.at(uv.y)) /
              (top_level.column_sum * columns.at(uv.x).column_sum) * 
              ( 1.0 / (2.0 * M_PI * M_PI * sin(theta)));
    return pdf;
}

auto EnvironmentMap::sample_direction() -> f64vec3
{
    f64 rand_one = get_random_double();
    f64 rand_two = get_random_double();
    auto row_sample = top_level.sample(rand_one);

    i32 row_idx = glm::clamp(i32(row_sample.sample), 0, i32(top_level.CDF.size() - 2));
    auto column_sample = columns.at(row_idx).sample(rand_two);


    f64 theta = column_sample.sample / (columns.at(row_idx).CDF.size() - 2) * M_PI;
    f64 phi = row_sample.sample / (top_level.CDF.size() - 2) * M_PI * 2.0f;
    f64 cos_theta = glm::cos(theta);
    f64 sin_theta = glm::sin(theta);
    f64 cos_phi = glm::cos(phi);
    f64 sin_phi = glm::sin(phi);

    f64vec3 out_dir = f64vec3(sin_theta * cos_phi, sin_theta * sin_phi, cos_theta);
    // out_dir = glm::normalize(out_dir);
    return out_dir;
}

Scene::Scene(const Camera & camera) : camera{camera}, total_power{0.0}, use_env_map{true}
{
}

void Scene::calculate_total_power()
{
    total_power = 0.0;
    for(const auto & object : scene_objects)
    {
        total_power += std::visit(GetPower{}, object);
    }
    std::cout << "total scene power : " << total_power << std::endl;
}

void Scene::load_scene_from_file()
{
    throw std::runtime_error("[Scene::load_scene_from_file()] not yet implemented");
}
void Scene::save_scene_to_file()
{
    throw std::runtime_error("[Scene::save_scene_to_file()] not yet implemented");
}