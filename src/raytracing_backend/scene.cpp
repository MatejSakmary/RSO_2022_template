#include "scene.hpp"

#include <iostream>

void EnvironmentMap::init()
{
    lum_image = std::vector<float>(width * height);
    heat_map = std::vector<float>(width * height, 0.0f);
    col_prob = std::vector<float>(width);
    total_power = 0.0f;
    for(size_t x = 0; x < width; x++)
    {
        float total_col_intensity = 0;
        for(size_t y = 0; y < height; y++)
        {
            // y = 0        -> angle = 0
            // y = height/2 -> angle = pi/2
            // y = height   -> angle = pi
            float uv = (float(y) + 0.5f) / float(height);
            // spherical env map is set up in such a way that the first row is the top
            // of sphere. This than has the same area as the row in the middle of the sphere
            // thus we need to normalize by sin factor
            float norm_factor = sin(3.14159265 * uv);

            int a = y * width;
            int b = a + x;
            int c = b * 3;
            int idx_ = ((y * u32(width)) + x) * 3;
            if(c != idx_)
            {
                std::cout << "(y * width + x) * 3 where y: " << y << " width: " << u32(width) << " x: " << x << " results in " << idx_ << std::endl;
            }

            lum_image.at(b) = ( 0.2126f * image.at(c) + 0.7152f * image.at(c + 1) + 0.0722f * image.at(c + 2)) * norm_factor;
            total_power += lum_image.at(b);
            total_col_intensity += lum_image.at(b);
        }
        col_prob.at(x) = total_col_intensity;
    }

    // for(int i = 0; i < 100000; i++)
    // {
    //     float rand_col = get_random_double() * total_power;
    //     float col_num = 0.0f;
    //     float running = 0.0f;
    //     for(size_t j = 0; j < col_prob.size(); j++ )
    //     {
    //         running += col_prob.at(j);
    //         if(running > rand_col)
    //         {
    //             col_num = j;
    //             break;
    //         }
    //     }

    //     float rand_row = get_random_double() * col_prob.at(col_num);
    //     running = 0.0f;
    //     for(size_t j = 0; j < height; j++)
    //     {
    //         running += lum_image.at(width * j + col_num);
    //         if(running > rand_row)
    //         {

    //             image.at((j * width + col_num) * 3) = 1.0f;
    //             heat_map.at(j * width + col_num) = 1.0f;
    //             break;
    //         }
    //     }
    // }
    std::cout << "Total power: " << total_power << std::endl;
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