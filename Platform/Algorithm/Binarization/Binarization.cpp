//
// Created by Ext Culith on 2025/7/26.
//

#include "Binarization.h"

#include <iostream>
#include <Core/Logger/Logger.h>
#include <glm/glm.hpp>

namespace cyanvne::platform::algorithm::binarization
{
    void print_binarization_grid(const BinarizationResult& result)
    {
        if (result.grid.empty())
        {
            cyanvne::core::GlobalLogger::getCoreLogger()->error("Binarization: Result grid is empty, nothing to print.");
            return;
        }
        for (int y = 0; y < result.height; ++y)
        {
            for (int x = 0; x < result.width; ++x)
            {
                std::cout << (result.grid[y * result.width + x] ? "#" : ".");
            }

            std::cout << std::endl;
        }
    }

    BinarizationResult binarize_from_single_channel(
        SDL_Surface* source_surface, BinarizationChannel channel, uint8_t threshold, bool done_release)
    {
        if (!source_surface)
        {
            cyanvne::core::GlobalLogger::getCoreLogger()->error("Binarization: Cannot binarize from a null surface.");
            return {};
        }

        // Convert the surface to a format that supports RGBA32
        SDL_Surface* formatted_surface = SDL_ConvertSurface(source_surface, SDL_PIXELFORMAT_RGBA32);
        if (!formatted_surface)
        {
            cyanvne::core::GlobalLogger::getCoreLogger()->error("Binarization: Failed to convert surface: {}", SDL_GetError());
            return {};
        }

        BinarizationResult result;
        result.width = formatted_surface->w;
        result.height = formatted_surface->h;
        result.grid.resize(result.width * result.height, false);

        if (SDL_MUSTLOCK(formatted_surface))
        {
            SDL_LockSurface(formatted_surface);
        }

        auto* pixels = static_cast<uint8_t*>(formatted_surface->pixels);
        int pitch = formatted_surface->pitch;
        int bytesPerPixel = SDL_BYTESPERPIXEL(formatted_surface->format);

        int channel_offset;
        switch (channel)
        {
        case BinarizationChannel::Red:
            channel_offset = 0;
            break;
        case BinarizationChannel::Green:
            channel_offset = 1;
            break;
        case BinarizationChannel::Blue:
            channel_offset = 2;
            break;
        case BinarizationChannel::Alpha:
            channel_offset = 3;
            break;
        default:
            cyanvne::core::GlobalLogger::getCoreLogger()->error("Binarization: Invalid channel specified.");
            if (done_release)
            {
                SDL_DestroySurface(formatted_surface);
            }
            return {};
        }

        for (int y = 0; y < result.height; ++y)
        {
            for (int x = 0; x < result.width; ++x)
            {
                uint8_t* current_pixel = pixels + y * pitch + x * bytesPerPixel;
                uint8_t channel_value = current_pixel[channel_offset];

                if (channel_value > threshold)
                {
                    result.grid[y * result.width + x] = true;
                }
            }
        }

        if (SDL_MUSTLOCK(formatted_surface))
            SDL_UnlockSurface(formatted_surface);

        if (done_release)
        {
            SDL_DestroySurface(formatted_surface);
        }

        return result;
    }

    float calculate_otsu_threshold(SDL_Surface* source_surface, LuminanceMethod method)
    {
        if (!source_surface)
        {
            cyanvne::core::GlobalLogger::getCoreLogger()->error("Otsu's threshold calculation: Source surface is null.");
            return 0.5f;
        }

        SDL_Surface* formatted_surface = SDL_ConvertSurface(source_surface, SDL_PIXELFORMAT_RGBA32);
        if (!formatted_surface)
        {
            cyanvne::core::GlobalLogger::getCoreLogger()->error("Otsu's threshold calculation: Failed to convert surface: {}", SDL_GetError());
            return 0.5f;
        }

        std::vector<float> luminance_map(formatted_surface->w * formatted_surface->h);
        if (SDL_MUSTLOCK(formatted_surface))
        {
            SDL_LockSurface(formatted_surface);
        }

        uint8_t* pixels = static_cast<uint8_t*>(formatted_surface->pixels);
        int pitch = formatted_surface->pitch;
        int bytes_per_pixel = SDL_BYTESPERPIXEL(formatted_surface->format);

        for (int y = 0; y < formatted_surface->h; ++y)
        {
            for (int x = 0; x < formatted_surface->w; ++x)
            {
                uint8_t* p = pixels + y * pitch + x * bytes_per_pixel;
                float r_norm = p[0] / 255.0f;
                float g_norm = p[1] / 255.0f;
                float b_norm = p[2] / 255.0f;

                float luminance = 0.0f;
                if (method == LuminanceMethod::Perceptual)
                {
                    float r_linear = std::pow(r_norm, 2.2f);
                    float g_linear = std::pow(g_norm, 2.2f);
                    float b_linear = std::pow(b_norm, 2.2f);
                    luminance = 0.2126f * r_linear + 0.7152f * g_linear + 0.0722f * b_linear;
                }
                else // Quick method
                {
                    luminance = 0.299f * r_norm + 0.587f * g_norm + 0.114f * b_norm;
                }
                luminance_map[y * formatted_surface->w + x] = luminance;
            }
        }

        if (SDL_MUSTLOCK(formatted_surface)) SDL_UnlockSurface(formatted_surface);
        SDL_DestroySurface(formatted_surface);

        std::vector<int> histogram(256, 0);
        for (float lum : luminance_map)
        {
            uint8_t lum_int = static_cast<uint8_t>(glm::clamp(lum * 255.0f, 0.0f, 255.0f));
            histogram[lum_int]++;
        }

        long long total_pixels = formatted_surface->w * formatted_surface->h;
        float sum_all = 0;
        for (int i = 0; i < 256; ++i) sum_all += i * histogram[i];

        float max_variance = 0.0f;
        int optimal_threshold_int = 0;
        long long w_b = 0;
        float sum_b = 0;

        for (int t = 0; t < 256; ++t)
        {
            w_b += histogram[t];

            if (w_b == 0)
                continue;

            long long w_f = total_pixels - w_b;

            if (w_f == 0)
                break;

            sum_b += (float)(t * histogram[t]);

            float mean_b = sum_b / w_b;
            float mean_f = (sum_all - sum_b) / w_f;
            float variance_between = static_cast<double>(w_b) * static_cast<double>(w_f) * (mean_b - mean_f) * (mean_b - mean_f);

            if (variance_between > max_variance)
            {
                max_variance = variance_between;
                optimal_threshold_int = t;
            }
        }

        return optimal_threshold_int / 255.0f;
    }

    BinarizationResult binarize_from_luminance(
            SDL_Surface* source_surface, LuminanceMethod method, float threshold, bool done_release)
    {
        if (!source_surface)
        {
            cyanvne::core::GlobalLogger::getCoreLogger()->error("Binarization: Source surface is null.");
            return {};
        }

        SDL_Surface* formatted_surface = SDL_ConvertSurface(source_surface, SDL_PIXELFORMAT_RGBA32);
        if (!formatted_surface)
        {
            cyanvne::core::GlobalLogger::getCoreLogger()->error("Binarization: Failed to convert surface: {}", SDL_GetError());
            return {};
        }

        BinarizationResult result;
        result.width = formatted_surface->w;
        result.height = formatted_surface->h;
        result.grid.resize(result.width * result.height);

        if (SDL_MUSTLOCK(formatted_surface))
        {
            SDL_LockSurface(formatted_surface);
        }

        uint8_t* pixels = static_cast<uint8_t*>(formatted_surface->pixels);
        int pitch = formatted_surface->pitch;
        int bytes_per_pixel = SDL_BYTESPERPIXEL(formatted_surface->format);

        for (int y = 0; y < result.height; ++y)
        {
            for (int x = 0; x < result.width; ++x)
            {
                uint8_t* p = pixels + y * pitch + x * bytes_per_pixel;
                float r_norm = p[0] / 255.0f;
                float g_norm = p[1] / 255.0f;
                float b_norm = p[2] / 255.0f;

                float luminance = 0.0f;
                if (method == LuminanceMethod::Perceptual)
                {
                    float r_linear = std::pow(r_norm, 2.2f);
                    float g_linear = std::pow(g_norm, 2.2f);
                    float b_linear = std::pow(b_norm, 2.2f);
                    luminance = 0.2126f * r_linear + 0.7152f * g_linear + 0.0722f * b_linear;
                }
                else // Quick
                {
                    luminance = 0.299f * r_norm + 0.587f * g_norm + 0.114f * b_norm;
                }

                result.grid[y * result.width + x] = (luminance > threshold);
            }
        }

        if (SDL_MUSTLOCK(formatted_surface))
        {
            SDL_UnlockSurface(formatted_surface);
        }

        if (done_release)
        {
            SDL_DestroySurface(formatted_surface);
        }

        return result;
    }
}
