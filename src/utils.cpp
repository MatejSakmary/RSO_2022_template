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

auto load_exr_image(const std::string & path, std::vector<float> & img) -> void
{
    EXRVersion exr_version;
    if(ParseEXRVersionFromFile(&exr_version, path.c_str()) != 0)
    {
        throw std::runtime_error("[load_exr_image()] invalid exr file");
    }
    if(exr_version.multipart) 
    { 
        throw std::runtime_error("[load_exr_image()] exr file must not be multipart");
    }

    EXRHeader exr_header;
    InitEXRHeader(&exr_header);
    const char * err = nullptr;
    if(ParseEXRHeaderFromFile(&exr_header, &exr_version, path.c_str(), &err) != 0)
    {
        std::string err_message(err);
        FreeEXRErrorMessage(err);
        throw std::runtime_error("[load_exr_image()] failed to load exr header : " + err_message);
    } 

    EXRImage exr_image;
    InitEXRImage(&exr_image);
    if(LoadEXRImageFromFile(&exr_image, &exr_header, path.c_str(), &err) != 0)
    {
        std::string err_message(err);
        FreeEXRHeader(&exr_header);
        FreeEXRErrorMessage(err);
        throw std::runtime_error("[load_exr_image()] failed to load exr image : " + err_message);
    }

    img.clear();
    img.reserve(exr_image.width * exr_image.height * exr_image.num_channels * sizeof(float));

    for (int i = 0; i < exr_image.width * exr_image.height; i++) 
    {
        img.at(4 * i + 0) = reinterpret_cast<float **>(exr_image.images)[0][i];
        img.at(4 * i + 1) = reinterpret_cast<float **>(exr_image.images)[1][i];
        img.at(4 * i + 2) = reinterpret_cast<float **>(exr_image.images)[2][i];
    }
}