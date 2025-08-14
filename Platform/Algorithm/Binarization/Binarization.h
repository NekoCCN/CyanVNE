//
// Created by Ext Culith on 2025/7/26.
//

#ifndef BINARIZATION_H
#define BINARIZATION_H

#include <vector>
#include <SDL3/SDL_surface.h>

namespace cyanvne::platform::algorithm::binarization
{
    struct BinarizationResult
    {
        // Notice: std::vector<bool> is a bitset, which is more memory efficient for binary data.
        std::vector<bool> grid;
        int width = 0;
        int height = 0;
    };

    enum class BinarizationChannel
    {
        Red,
        Green,
        Blue,
        Alpha
    };

    enum class LuminanceMethod
    {
        // Quick Luminance method: uses a simple average of RGB values, fast but less accurate.
        // It is suitable for quick operations where performance is more critical than accuracy, eg: Runtime.
        // It base on the assumption that all colors contribute equally to perceived brightness.
        Quick,
        // Standard Luminance method: uses the standard formula for perceived brightness.
        // It base on the human eye's sensitivity to different colors.
        Perceptual
    };

    /**
     * Prints the binarization grid to the console.
     * @param result The BinarizationResult containing the grid to print.
     */
    void print_binarization_grid(const BinarizationResult& result);

    /**
     * Binarizes an SDL_Surface based on a single channel (Red, Green, Blue, or Alpha).
     * @param source_surface The SDL_Surface to binarize.
     * @param channel The channel to use for binarization.
     * @param threshold The threshold value for binarization (default is 128).
     * @param done_release If true, the function will release the surface after processing (default is false).
     * @return A BinarizationResult containing the binarized grid and dimensions.
     */
    BinarizationResult binarize_from_single_channel(SDL_Surface* source_surface,
        BinarizationChannel channel = BinarizationChannel::Alpha, uint8_t threshold = 128, bool done_release = false);

    /**
     * Calculates the Otsu's threshold for binarization from an SDL_Surface.
     * @param source_surface The SDL_Surface to analyze.
     * @param method The luminance method to use (Quick or Perceptual).
     * @return The calculated Otsu's threshold value.
     */
    float calculate_otsu_threshold(SDL_Surface* source_surface, LuminanceMethod method);

    /**
     * Binarizes an SDL_Surface based on luminance values.
     * @param source_surface The SDL_Surface to binarize.
     * @param method The luminance method to use (Quick or Perceptual).
     * @param threshold The threshold value for binarization (default is 0.5).
     * @param done_release If true, the function will release the surface after processing (default is false).
     * @return A BinarizationResult containing the binarized grid and dimensions.
     */
    BinarizationResult binarize_from_luminance(
       SDL_Surface* source_surface, LuminanceMethod method, float threshold, bool done_release = false);
}

#endif //BINARIZATION_H
