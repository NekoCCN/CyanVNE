#include "ResourceTypes.h"
#include "Resources/ResourcesManager/ResourcesManager.h"
#include "Resources/ResourcesException/ResourcesException.h"
#include <SDL3/SDL.h>
#include <SDL3_image/SDL_image.h>
#include "soloud.h"
#include "soloud_wav.h"
#include <stdexcept>

namespace cyanvne
{
    namespace resources
    {
        RawDataResource::RawDataResource(uint64_t id, const ResourcesManager* base_manager)
        {
            data = base_manager->getResourceDataById(id);
        }

        size_t RawDataResource::getSizeInBytes() const
        {
            return data.size();
        }

        TextureResource::TextureResource(const std::vector<uint8_t>& raw_data, SDL_Renderer* renderer)
        {
            if (!renderer)
            {
                throw std::invalid_argument("Renderer is null.");
            }

            SDL_IOStream* stream = SDL_IOFromConstMem(raw_data.data(), static_cast<int>(raw_data.size()));
            if (!stream)
            {
                throw exception::resourcesexception::ResourceManagerIOException("Failed to create RWops from memory.");
            }

            texture = IMG_LoadTexture_IO(renderer, stream, true);
            if (!texture)
            {
                throw exception::resourcesexception::ResourceManagerIOException(
                    "Failed to load texture from data: " + std::string(SDL_GetError()));
            }
        }

        TextureResource::~TextureResource()
        {
            if (texture)
            {
                SDL_DestroyTexture(texture);
            }
        }

        size_t TextureResource::getSizeInBytes() const
        {
            if (!texture) return 0;

            SDL_PropertiesID props = SDL_GetTextureProperties(texture);

            int width = static_cast<int>(SDL_GetNumberProperty(props, SDL_PROP_TEXTURE_WIDTH_NUMBER, 0));
            int height = static_cast<int>(SDL_GetNumberProperty(props, SDL_PROP_TEXTURE_HEIGHT_NUMBER, 0));
            
            Uint32 format = static_cast<Uint32>(SDL_GetNumberProperty(props, SDL_PROP_TEXTURE_FORMAT_NUMBER, 0));

            return static_cast<size_t>(width) * height * SDL_BYTESPERPIXEL(format);
        }

        SoLoudWavResource::SoLoudWavResource(const std::vector<uint8_t>& raw_data)
        {
            SoLoud::result res = sound.loadMem(
                const_cast<unsigned char*>(raw_data.data()),
                static_cast<unsigned int>(raw_data.size()),
                false,
                false
            );

            if (res != SoLoud::SO_NO_ERROR)
            {
                throw exception::resourcesexception::ResourceManagerIOException(
                    "Failed to load SoLoud Wav from memory.");
            }
            decoded_size_bytes_ = static_cast<size_t>(sound.mSampleCount) * sizeof(float) * sound.mChannels;
        }

        size_t SoLoudWavResource::getSizeInBytes() const
        {
            return decoded_size_bytes_;
        }
    }
}
