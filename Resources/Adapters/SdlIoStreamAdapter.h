#pragma once
#include <SDL3/SDL.h>
#include <Core/Stream/Stream.h> // 假设这是您项目的流头文件
#include <memory>

namespace cyanvne
{
    namespace resources
    {
        namespace adapters
        {
            struct SdlInStreamAdapterContext
            {
                std::shared_ptr<core::stream::InStreamInterface> stream;
            };
            struct SdlOutStreamAdapterContext
            {
                std::shared_ptr<core::stream::OutStreamInterface> stream;
            };
            struct SdlBiStreamAdapterContext
            {
                std::shared_ptr<core::stream::StreamInterface> stream;
            };

            Sint64 SDLCALL sdl_size_in_cb(void* userdata);
            Sint64 SDLCALL sdl_seek_in_cb(void* userdata, Sint64 offset, SDL_IOWhence whence);
            size_t SDLCALL sdl_read_in_cb(void* userdata, void* ptr, size_t size, SDL_IOStatus* status);
            size_t SDLCALL sdl_write_in_cb(void* userdata, const void* ptr, size_t size, SDL_IOStatus* status);
            bool SDLCALL sdl_close_in_cb(void* userdata);

            Sint64 SDLCALL sdl_size_out_cb(void* userdata);
            Sint64 SDLCALL sdl_seek_out_cb(void* userdata, Sint64 offset, SDL_IOWhence whence);
            size_t SDLCALL sdl_write_out_cb(void* userdata, const void* ptr, size_t size, SDL_IOStatus* status);
            bool SDLCALL sdl_close_out_cb(void* userdata);

            Sint64 SDLCALL sdl_size_bi_cb(void* userdata);
            Sint64 SDLCALL sdl_seek_bi_cb(void* userdata, Sint64 offset, SDL_IOWhence whence);
            size_t SDLCALL sdl_read_bi_cb(void* userdata, void* ptr, size_t size, SDL_IOStatus* status);
            size_t SDLCALL sdl_write_bi_cb(void* userdata, const void* ptr, size_t size, SDL_IOStatus* status);
            bool SDLCALL sdl_close_bi_cb(void* userdata);

            SDL_IOStream* CreateSdlIoStreamFromCyanvneInStream(std::shared_ptr<core::stream::InStreamInterface> cyanvne_stream);
            SDL_IOStream* CreateSdlIoStreamFromCyanvneOutStream(std::shared_ptr<core::stream::OutStreamInterface> cyanvne_stream);
            SDL_IOStream* CreateSdlIoStreamFromCyanvneStream(std::shared_ptr<core::stream::StreamInterface> cyanvne_stream);
        }
    }
}