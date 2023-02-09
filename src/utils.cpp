#include "utils.hpp"
#include "string.h"
#include <iostream>
#include <utility>
#include <functional>

static std::mt19937 engine = std::mt19937(123);
static std::uniform_real_distribution distribution = std::uniform_real_distribution<f64>(0.0, 1.0);

auto save_hdr_image(const std::string & path, std::vector<float> & image, i32 width, i32 height) -> void
{
#if defined(_WIN32)
    FILE* fp;
    auto err = fopen_s(&fp, path.c_str(), "wb");
#else
    auto fp = fopen(path.c_str(), "wb");
#endif
    if (fp)
    {
        size_t nmemb = width * height;
        typedef unsigned char RGBE[4];
        RGBE *data = new RGBE[nmemb];
        for (int ii = 0; ii < nmemb; ii++)
        {
            RGBE &rgbe = data[ii];
            int x = (ii % width);
            int y = height - (ii / width) - 1;
            f32vec3 vv;
            vv = reinterpret_cast<f32vec3*>(image.data())[y * width + x];
            float v;
            int e;
            v = vv.x;
            if (vv.y > v)
                v = vv.y;
            if (vv.z > v)
                v = vv.z;
            if (v < 1e-32)
            {
                rgbe[0] = rgbe[1] = rgbe[2] = rgbe[3] = 0x0;
            }
            else
            {
                v = (float)(frexp(v, &e) * 256.0 / v);
                rgbe[0] = (unsigned char)(vv.x * v);
                rgbe[1] = (unsigned char)(vv.y * v);
                rgbe[2] = (unsigned char)(vv.z * v);
                rgbe[3] = (unsigned char)(e + 128);
            }
        }
        fflush(stdout);
        const char *programtype = "RADIANCE";
        if (fprintf(fp, "#?%s\n", programtype) < 0)
        {
            abort();
        }
        float gamma = 2.2;
        float exposure = 1.0;
        if (fprintf(fp, "GAMMA=%g\n", gamma) < 0)
        {
            abort();
        }
        if (fprintf(fp, "EXPOSURE=%g\n", exposure) < 0)
        {
            abort();
        }
        if (fprintf(fp, "FORMAT=32-bit_rle_rgbe\n\n") < 0)
        {
            abort();
        }
        if (fprintf(fp, "-Y %d +X %d\n", height, width) < 0)
        {
            abort();
        }
        // Write data
        size_t kk = fwrite(data, (size_t)4, nmemb, fp);
        fclose(fp);
        if (kk != nmemb)
        {
            std::cout << "ERROR - was not able to save all kk= " << (int)nmemb << "entries to file, exiting" << std::endl;
            fflush(stdout);
            abort(); // error
        }
    }
}

f64 get_random_double()
{
    // return (double)rand() / RAND_MAX;
    return distribution(engine);
}

f64vec3 get_random_double_vec()
{
    return {get_random_double(), get_random_double(), get_random_double()};
}

#pragma region hdr_loading
const u64 MINELEN = 8;
const u64 MAXELEN = 0x7fff;

struct HDR_Pixel
{
    char R;
    char G;
    char B;
    char E;

    auto at(u32 i) -> char &
    {
        switch (i)
        {
        case 0:
            return R;
        case 1:
            return G;
        case 2:
            return B;
        case 3:
            return E;
        default:
            return E;
        }
    }
};


auto old_decrunch(std::vector<HDR_Pixel> & line, std::ifstream & hdr_file, bool first_occupied = false) -> void
{
    int rshift = 0;
    for(int i = first_occupied ? 1 : 0; i < line.size();)
    {
        if(hdr_file.peek() == EOF) { throw std::runtime_error("[old_decrunch()] TODO: Msakmary fix this"); }
        hdr_file.read(reinterpret_cast<char*>(&line.at(i).R), 1);
        hdr_file.read(reinterpret_cast<char*>(&line.at(i).G), 1);
        hdr_file.read(reinterpret_cast<char*>(&line.at(i).B), 1);
        hdr_file.read(reinterpret_cast<char*>(&line.at(i).E), 1);

        if (line.at(i).R == 1 &&
            line.at(i).G == 1 &&
            line.at(i).B == 1)
        {
            for(int j = line.at(i).E << rshift; j > 0; j--)
            {
                memcpy(&line.at(i).R, &line.at(i - 1).R, 4);
                i++;
            }
            rshift += 8;
        }
        else 
        {
            i++;
            rshift = 0;
        }
    }
}

auto hdr_decrunch(std::vector<HDR_Pixel> & line, std::ifstream & hdr_file) -> void
{
    if(line.size() < MINELEN || line.size() > MAXELEN)
    {
        old_decrunch(line, hdr_file);
        return;
    }
    char c;
    hdr_file.read(&c, 1);
    if(c != 2)
    {
        hdr_file.seekg(-1, std::ios_base::cur);
        old_decrunch(line, hdr_file);
        return;
    }
    hdr_file.read(&line.at(0).G, 1);
    hdr_file.read(&line.at(0).B, 1);
    hdr_file.read(&c, 1);
    if(line.at(0).G != 2 || line.at(0).B & 128)
    {
        line.at(0).R = 2;
        line.at(0).E = c;
        old_decrunch(line, hdr_file, true);
        return;
    }

    for (int i = 0; i < 4; i++)
    {
        for (int j = 0; j < line.size(); )
        {
            unsigned char code; 
            hdr_file.read(reinterpret_cast<char*>(&code), 1);
            std::cout << int(code) << std::endl;
            if(code > 128)
            {
                code &= 127;
                unsigned char val; 
                hdr_file.read(reinterpret_cast<char*>(&val), 1);
                while (code--) { line.at(j++).at(i) = val; }
            } else
            {
                while(code--)
                {
                    hdr_file.read(reinterpret_cast<char*>(&line.at(j++).at(i)), 1);
                }
            }
        }
    }
    if(hdr_file.peek() != EOF) { throw std::runtime_error("[decrunch()] TODO: Msakmary fix this"); }
}

auto convert_component(int expo, int val) -> float
{
    float v = val / 256.0f;
	float d = (float) pow(2, expo);
	return v * d;
}
auto work_on_rgbe(std::vector<HDR_Pixel> & row, std::vector<float> & image) -> void
{
    for(int i = 0; i < row.size(); i++)
    {
        int exposure = static_cast<unsigned char>(row.at(i).E) - 128;
        image.emplace_back(convert_component(exposure, static_cast<unsigned char>(row.at(i).R)));
        image.emplace_back(convert_component(exposure, static_cast<unsigned char>(row.at(i).G)));
        image.emplace_back(convert_component(exposure, static_cast<unsigned char>(row.at(i).B)));
    }
}

auto load_hdr_image(const std::string & path, std::vector<float> & image, i32 & width, i32 & height) -> void
{
    image.clear();
    char buff[200];
    std::ifstream hdr_file(path, std::ios::binary);
    if(!hdr_file) { throw std::runtime_error("[load_hdr_image()] Failed to open file " + path); }

    hdr_file.read(buff, 10);
    if(memcmp(buff, "#?RADIANCE", 10))
    {
        throw std::runtime_error("[load_hdr_image()] Invalid hdr header");
    }
    hdr_file.seekg(1, std::ios_base::cur);

    // read until we find two LF symbols
    for(char c = 0, c_old = 0; !(c == 0xa && c_old == 0xa);)
    {
        c_old = c;
        hdr_file.read(&c, 1);
    } 

    int j = 0;
    for(int i = 0; ;i++)
    {
        hdr_file.read(&buff[i], 1);
        if(buff[i] == 0xa) { break; }
    }

    i64 size_x;
    i64 size_y;
#if defined(_WIN32)
    if(!sscanf_s(buff, "-Y %lld +X %lld", &size_y, &size_x))
#else
    if(!sscanf(buff, "-Y %ld +X %ld", &size_y, &size_x))
#endif
    {
        hdr_file.close();
        throw std::runtime_error("[laod_hdr_image()] Invalid header info");
    }
    width = size_x;
    height = size_y;

    image.reserve(size_x * size_y * 3);
    std::vector<HDR_Pixel> row(size_x);
    for(int y = size_y - 1; y >= 0; y--)
    {
        hdr_decrunch(row, hdr_file);
        work_on_rgbe(row, image);
    }
    row.clear();
}