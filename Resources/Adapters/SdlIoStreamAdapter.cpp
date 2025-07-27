#include "SdlIoStreamAdapter.h"

namespace cyanvne::resources::adapters
{
    Sint64 SDLCALL sdl_size_in_cb(void* userdata)
    {
        auto* context = static_cast<SdlInStreamAdapterContext*>(userdata);

        if (!context || !context->stream)
            return -1;

        auto* stream = context->stream.get();

        int64_t current_pos = stream->tell();

        if (current_pos < 0)
            return -1;

        int64_t size = stream->seek(0, core::stream::SeekMode::End);

        stream->seek(current_pos, core::stream::SeekMode::Begin);

        return size;
    }

    Sint64 SDLCALL sdl_seek_in_cb(void* userdata, Sint64 offset, SDL_IOWhence whence)
    {
        auto* context = static_cast<SdlInStreamAdapterContext*>(userdata);

        if (!context || !context->stream)
            return -1;

        core::stream::SeekMode mode;

        switch (whence)
        {
        case SDL_IO_SEEK_SET:
            mode = core::stream::SeekMode::Begin;
            break;
        case SDL_IO_SEEK_CUR:
            mode = core::stream::SeekMode::Current;
            break;
        case SDL_IO_SEEK_END:
            mode = core::stream::SeekMode::End;
            break;
        default:
            return -1;
        }

        return context->stream->seek(offset, mode);
    }

    size_t SDLCALL sdl_read_in_cb(void* userdata, void* ptr, size_t size, SDL_IOStatus* status)
    {
        auto* context = static_cast<SdlInStreamAdapterContext*>(userdata);

        if (!context || !context->stream)
        {
            if (status)
                *status = SDL_IO_STATUS_ERROR;
            return 0;
        }

        size_t bytes_read = context->stream->read(ptr, size);

        if (bytes_read < size)
        {
            if (context->stream->tell() >= sdl_size_in_cb(userdata))
            {
                if (status)
                    *status = SDL_IO_STATUS_EOF;
            }
            else
            {
                if (status)
                    *status = SDL_IO_STATUS_ERROR;
            }
        }
        else if (status)
        {
            *status = SDL_IO_STATUS_READY;
        }
        return bytes_read;
    }

    size_t SDLCALL sdl_write_in_cb(void* userdata, const void* ptr, size_t size, SDL_IOStatus* status)
    {
        if (status)
            *status = SDL_IO_STATUS_ERROR;
        SDL_SetError("This stream is read-only");
        return 0;
    }

    bool SDLCALL sdl_close_in_cb(void* userdata)
    {
        if (userdata)
        {
            delete static_cast<SdlInStreamAdapterContext*>(userdata);
        }
        return true;
    }

    Sint64 SDLCALL sdl_size_out_cb(void* userdata)
    {
        auto* context = static_cast<SdlOutStreamAdapterContext*>(userdata);
        if (!context || !context->stream)
            return -1;

        auto* stream = context->stream.get();
        int64_t current_pos = stream->tell();

        if (current_pos < 0)
            return -1;

        int64_t size = stream->seek(0, core::stream::SeekMode::End);
        stream->seek(current_pos, core::stream::SeekMode::Begin);

        return size;
    }

    Sint64 SDLCALL sdl_seek_out_cb(void* userdata, Sint64 offset, SDL_IOWhence whence)
    {
        auto* context = static_cast<SdlOutStreamAdapterContext*>(userdata);

        if (!context || !context->stream)
            return -1;

        core::stream::SeekMode mode;

        switch (whence)
        {
        case SDL_IO_SEEK_SET:
            mode = core::stream::SeekMode::Begin;
            break;
        case SDL_IO_SEEK_CUR:
            mode = core::stream::SeekMode::Current;
            break;
        case SDL_IO_SEEK_END:
            mode = core::stream::SeekMode::End;
            break;
        default:
            return -1;
        }

        return context->stream->seek(offset, mode);
    }

    size_t SDLCALL sdl_write_out_cb(void* userdata, const void* ptr, size_t size, SDL_IOStatus* status)
    {
        auto* context = static_cast<SdlOutStreamAdapterContext*>(userdata);
        if (!context || !context->stream)
        {
            if (status)
                *status = SDL_IO_STATUS_ERROR;
            return 0;
        }
        size_t bytes_written = context->stream->write(ptr, size);
        if (bytes_written < size)
        {
            if (status)
                *status = SDL_IO_STATUS_ERROR;
        }
        else
        {
            if (status)
                *status = SDL_IO_STATUS_READY;
        }
        return bytes_written;
    }

    bool SDLCALL sdl_close_out_cb(void* userdata)
    {
        if (userdata)
        {
            delete static_cast<SdlOutStreamAdapterContext*>(userdata);
        }
        return true;
    }

    Sint64 SDLCALL sdl_size_bi_cb(void* userdata)
    {
        auto* context = static_cast<SdlBiStreamAdapterContext*>(userdata);

        if (!context || !context->stream)
            return -1;

        auto* stream = context->stream.get();
        int64_t current_pos = stream->tell();

        if (current_pos < 0)
            return -1;

        int64_t size = stream->seek(0, core::stream::SeekMode::End);

        stream->seek(current_pos, core::stream::SeekMode::Begin);

        return size;
    }

    Sint64 SDLCALL sdl_seek_bi_cb(void* userdata, Sint64 offset, SDL_IOWhence whence)
    {
        auto* context = static_cast<SdlBiStreamAdapterContext*>(userdata);
        if (!context || !context->stream) return -1;
        core::stream::SeekMode mode;
        switch (whence)
        {
        case SDL_IO_SEEK_SET:
            mode = core::stream::SeekMode::Begin;
            break;
        case SDL_IO_SEEK_CUR:
            mode = core::stream::SeekMode::Current;
            break;
        case SDL_IO_SEEK_END:
            mode = core::stream::SeekMode::End;
            break;
        default:
            return -1;
        }

        return context->stream->seek(offset, mode);
    }

    size_t SDLCALL sdl_read_bi_cb(void* userdata, void* ptr, size_t size, SDL_IOStatus* status)
    {
        auto* context = static_cast<SdlBiStreamAdapterContext*>(userdata);
        if (!context || !context->stream)
        {
            if (status) *status = SDL_IO_STATUS_ERROR;
            return 0;
        }

        size_t bytes_read = context->stream->read(ptr, size);

        if (bytes_read < size)
        {
            if (context->stream->tell() >= sdl_size_bi_cb(userdata))
            {
                if (status) *status = SDL_IO_STATUS_EOF;
            }
            else
            {
                if (status) *status = SDL_IO_STATUS_ERROR;
            }
        }
        else if (status)
        {
            *status = SDL_IO_STATUS_READY;
        }
        return bytes_read;
    }

    size_t SDLCALL sdl_write_bi_cb(void* userdata, const void* ptr, size_t size, SDL_IOStatus* status)
    {
        auto* context = static_cast<SdlBiStreamAdapterContext*>(userdata);

        if (!context || !context->stream)
        {
            if (status) *status = SDL_IO_STATUS_ERROR;
            return 0;
        }
        size_t bytes_written = context->stream->write(ptr, size);
        if (bytes_written < size)
        {
            if (status)
                *status = SDL_IO_STATUS_ERROR;
        }
        else
        {
            if (status)
                *status = SDL_IO_STATUS_READY;
        }
        return bytes_written;
    }

    bool SDLCALL sdl_close_bi_cb(void* userdata)
    {
        if (userdata)
        {
            delete static_cast<SdlBiStreamAdapterContext*>(userdata);
        }
        return true;
    }

    SDL_IOStream* CreateSdlIoStreamFromCyanvneInStream(std::shared_ptr<core::stream::InStreamInterface> cyanvne_stream)
    {
        if (!cyanvne_stream || !cyanvne_stream->is_open())
            return nullptr;

        auto* context = new(std::nothrow) SdlInStreamAdapterContext { std::move(cyanvne_stream) };

        if (!context)
            return nullptr;

        SDL_IOStreamInterface iface;

        SDL_INIT_INTERFACE(&iface);

        iface.size = sdl_size_in_cb;
        iface.seek = sdl_seek_in_cb;
        iface.read = sdl_read_in_cb;
        iface.write = sdl_write_in_cb;
        iface.close = sdl_close_in_cb;

        return SDL_OpenIO(&iface, context);
    }

    SDL_IOStream* CreateSdlIoStreamFromCyanvneOutStream(std::shared_ptr<core::stream::OutStreamInterface> cyanvne_stream)
    {
        if (!cyanvne_stream || !cyanvne_stream->is_open())
            return nullptr;

        auto* context = new(std::nothrow) SdlOutStreamAdapterContext{ std::move(cyanvne_stream) };

        if (!context)
            return nullptr;

        SDL_IOStreamInterface iface;
        SDL_INIT_INTERFACE(&iface);

        iface.size = sdl_size_out_cb;
        iface.seek = sdl_seek_out_cb;
        iface.write = sdl_write_out_cb;
        iface.close = sdl_close_out_cb;
        iface.read = nullptr;

        return SDL_OpenIO(&iface, context);
    }

    SDL_IOStream* CreateSdlIoStreamFromCyanvneStream(std::shared_ptr<core::stream::StreamInterface> cyanvne_stream)
    {
        if (!cyanvne_stream || !cyanvne_stream->is_open())
            return nullptr;
        auto* context = new(std::nothrow) SdlBiStreamAdapterContext{ std::move(cyanvne_stream) };

        if (!context)
            return nullptr;

        SDL_IOStreamInterface iface;
        SDL_INIT_INTERFACE(&iface);

        iface.size = sdl_size_bi_cb;
        iface.seek = sdl_seek_bi_cb;
        iface.read = sdl_read_bi_cb;
        iface.write = sdl_write_bi_cb;
        iface.close = sdl_close_bi_cb;

        return SDL_OpenIO(&iface, context);
    }
}