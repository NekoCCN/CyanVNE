#pragma once
#include <Resources/ICachedResource/ICachedResource.h>
#include <vector>
#include <Resources/ResourcesManager/ResourcesManager.h>
#include <soloud_wav.h>
#include <SDL3/SDL.h>

namespace cyanvne
{
    namespace resources
    {
        class RawDataResource : public ICachedResource
        {
        public:
            std::vector<uint8_t> data;
            explicit RawDataResource(uint64_t id, const ResourcesManager* base_manager);
            size_t getSizeInBytes() const override;
        };

        class TextureResource : public ICachedResource
        {
        public:
            SDL_Texture* texture = nullptr;
            explicit TextureResource(const std::vector<uint8_t>& raw_data, SDL_Renderer* renderer);
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
