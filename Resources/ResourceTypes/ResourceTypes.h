#pragma once
#include <Resources/ICachedResource/ICachedResource.h>
#include <vector>
#include <Resources/ResourcesManager/ResourcesManager.h>
#include <soloud_wav.h>
#include <bgfx/bgfx.h>

namespace cyanvne
{
    namespace resources
    {
        enum class ImageLoader
        {

            // Bimg
            INTERNAL,
            // SDL_Image
            EXTENDED
        };

        class RawDataResource : public ICachedResource
        {
        public:
            std::vector<uint8_t> data;
            explicit RawDataResource(uint64_t id, const ResourcesManager* base_manager);
            size_t getSizeInBytes() const override;
        };

        class TextureResource : public ICachedResource
        {
        private:
            uint32_t texture_size_bytes_ = 0;
        public:
            bgfx::TextureHandle texture_handle = BGFX_INVALID_HANDLE;

            explicit TextureResource(const std::vector<uint8_t>& raw_data,
                                     ImageLoader loader = ImageLoader::INTERNAL);
            ~TextureResource() override;
            size_t getSizeInBytes() const override;
        };

        class SoLoudWavResource : public ICachedResource
        {
        public:
            SoLoud::Wav sound;
            size_t decoded_size_bytes_ = 0;
            explicit SoLoudWavResource(const std::vector<uint8_t>& raw_data);
            size_t getSizeInBytes() const override;
        };
    }
}