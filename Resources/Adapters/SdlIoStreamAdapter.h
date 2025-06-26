#pragma once
#include <SDL3/SDL.h>
#include <Core/Stream/Stream.h>
#include <memory>

namespace cyanvne
{
    namespace resources
    {
        namespace adapters
        {
            struct SdlAdapterContext
            {
                std::shared_ptr<core::stream::InStreamInterface> stream;
            };

            Sint64 SDLCALL sdl_size_cb(void* userdata);

            Sint64 SDLCALL sdl_seek_cb(void* userdata, Sint64 offset, SDL_IOWhence whence);

            size_t SDLCALL sdl_read_cb(void* userdata, void* ptr, size_t size, SDL_IOStatus* status);

            size_t SDLCALL sdl_write_cb(void* userdata, const void* ptr, size_t size, SDL_IOStatus* status);

            bool SDLCALL sdl_close_cb(void* userdata);

            SDL_IOStream* CreateSdlIoStreamFromCyanvneStream(std::shared_ptr<core::stream::InStreamInterface> cyanvne_stream);
        }
    }
}