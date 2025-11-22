//
// Created by Ext CuliSth on 2025/9/1.
//

#include "Components.h"
#include "Resources/ResourceTypes/ResourceTypes.h"
#include "Resources/ResourcesException/ResourcesException.h"
#include <bimg/bimg.h>
#include <bx/readerwriter.h>

namespace cyanvne::runtime
{
    PinnedTexture::PinnedTexture(resources::PinnedResourceHandle&& handle)
            : data_handle(std::move(handle))
    {
        const auto& raw_data = data_handle.getData();
        if (raw_data.empty())
        {
            return;
        }

        bx::MemoryReader reader(raw_data.data(), raw_data.size());
        bx::Error err;
        bimg::ImageContainer* image_container = bimg::imageParse(&reader, &err);

        if (err.isOk() && image_container != nullptr)
        {
            this->width = image_container->m_width;
            this->height = image_container->m_height;

            if (bgfx::isTextureValid(0, false, image_container->m_numLayers,
                                     static_cast<bgfx::TextureFormat::Enum>(image_container->m_format), BGFX_TEXTURE_NONE))
            {
                this->texture_handle = bgfx::createTexture2D(
                        (uint16_t)this->width,
                        (uint16_t)this->height,
                        false,
                        image_container->m_numLayers,
                        static_cast<bgfx::TextureFormat::Enum>(image_container->m_format),
                        BGFX_TEXTURE_NONE | BGFX_SAMPLER_NONE,
                        bgfx::copy(image_container->m_data, image_container->m_size)
                );
            }
            bimg::imageFree(image_container);
        }

        if (!bgfx::isValid(this->texture_handle))
        {
        }
    }


    PinnedTexture::~PinnedTexture()
    {
        if (bgfx::isValid(texture_handle))
        {
            bgfx::destroy(texture_handle);
        }
    }

    // PinnedTexture 移动构造函数
    PinnedTexture::PinnedTexture(PinnedTexture&& other) noexcept
            : data_handle(std::move(other.data_handle)),
              texture_handle(other.texture_handle),
              width(other.width),
              height(other.height)
    {
        other.texture_handle = BGFX_INVALID_HANDLE;
        other.width = 0;
        other.height = 0;
    }

    // PinnedTexture 移动赋值运算符
    PinnedTexture& PinnedTexture::operator=(PinnedTexture&& other) noexcept
    {
        if (this != &other)
        {
            if (bgfx::isValid(texture_handle))
            {
                bgfx::destroy(texture_handle);
            }
            data_handle = std::move(other.data_handle);
            texture_handle = other.texture_handle;
            width = other.width;
            height = other.height;

            other.texture_handle = BGFX_INVALID_HANDLE;
            other.width = 0;
            other.height = 0;
        }
        return *this;
    }
}