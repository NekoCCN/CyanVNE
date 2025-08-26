#include "ResourceTypes.h"
#include "Resources/ResourcesManager/ResourcesManager.h"
#include "Resources/ResourcesException/ResourcesException.h"
#include <SDL3/SDL.h>
#include <SDL3_image/SDL_image.h>
#include "soloud.h"
#include "soloud_wav.h"
#include "bimg/bimg.h"
#include "bx/readerwriter.h"
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

        bgfx::TextureFormat::Enum toBgfxFormat(SDL_PixelFormat sdl_format)
        {
            switch (sdl_format)
            {
                case SDL_PIXELFORMAT_RGBA8888: return bgfx::TextureFormat::RGBA8;
                case SDL_PIXELFORMAT_BGRA8888: return bgfx::TextureFormat::BGRA8;
                case SDL_PIXELFORMAT_XRGB8888:   return bgfx::TextureFormat::RGB8;
                case SDL_PIXELFORMAT_RGB24:    return bgfx::TextureFormat::RGB8;
                default:                       return bgfx::TextureFormat::Unknown;
            }
        }

        TextureResource::TextureResource(const std::vector<uint8_t>& raw_data, ImageLoader loader)
        {
            if (raw_data.empty())
            {
                throw exception::resourcesexception::ResourceManagerIOException("Cannot create texture from empty data.");
            }

            if (loader == ImageLoader::INTERNAL)
            {
                bx::MemoryReader reader(raw_data.data(), raw_data.size());
                bx::Error err;

                bimg::ImageContainer image_container;

                if (!bimg::imageParse(image_container, &reader, &err))
                {
                    bimg::imageFree(&image_container);
                    throw exception::resourcesexception::ResourceManagerIOException("bimg failed to parse image data.");
                }

                if (bgfx::isTextureValid(0, false, image_container.m_numLayers,
                                         static_cast<bgfx::TextureFormat::Enum>(image_container.m_format), BGFX_TEXTURE_NONE))
                {
                    texture_handle = bgfx::createTexture2D(
                            uint16_t(image_container.m_width),
                            uint16_t(image_container.m_height),
                            false,
                            image_container.m_numLayers,
                            static_cast<bgfx::TextureFormat::Enum>(image_container.m_format),
                            BGFX_TEXTURE_NONE | BGFX_SAMPLER_NONE,
                            bgfx::copy(image_container.m_data, image_container.m_size)
                    );
                }

                texture_size_bytes_ = image_container.m_size;

                bimg::imageFree(&image_container);
            }
            else
            {
                SDL_IOStream* stream = SDL_IOFromConstMem(raw_data.data(), static_cast<int>(raw_data.size()));
                if (!stream)
                {
                    throw exception::resourcesexception::ResourceManagerIOException("Failed to create SDL_IOStream from memory.");
                }

                SDL_Surface* surface = IMG_Load_IO(stream, true);
                if (!surface)
                {
                    throw exception::resourcesexception::ResourceManagerIOException(
                            "SDL_image failed to load image: " + std::string(SDL_GetError()));
                }

                bgfx::TextureFormat::Enum format = toBgfxFormat(surface->format);
                if (format == bgfx::TextureFormat::Unknown)
                {
                    SDL_Surface* converted_surface = SDL_ConvertSurface(surface, SDL_PIXELFORMAT_RGBA8888);
                    SDL_DestroySurface(surface);
                    if (!converted_surface)
                    {
                        throw exception::resourcesexception::ResourceManagerIOException("Failed to convert surface to RGBA8888.");
                    }
                    surface = converted_surface;
                    format = bgfx::TextureFormat::RGBA8;
                }

                const bgfx::Memory* mem = bgfx::copy(surface->pixels, surface->w * surface->h * SDL_BYTESPERPIXEL(surface->format));
                texture_handle = bgfx::createTexture2D(
                        static_cast<uint16_t>(surface->w), static_cast<uint16_t>(surface->h),
                        false, 1, format, BGFX_TEXTURE_NONE | BGFX_SAMPLER_NONE, mem
                );

                texture_size_bytes_ = mem->size;
                SDL_DestroySurface(surface);
            }

            if (!bgfx::isValid(texture_handle))
            {
                throw exception::resourcesexception::ResourceManagerIOException("Failed to create a valid bgfx texture.");
            }
        }

        TextureResource::~TextureResource()
        {
            if (bgfx::isValid(texture_handle))
            {
                bgfx::destroy(texture_handle);
            }
        }

        size_t TextureResource::getSizeInBytes() const
        {
            return texture_size_bytes_;
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
