#include "SdlIoStreamAdapter.h"

Sint64 SDLCALL cyanvne::resources::adapters::sdl_size_cb(void* userdata)
{
    auto* context = static_cast<SdlAdapterContext*>(userdata);
    if (!context || !context->stream)
    {
        return -1;
    }
    auto* stream = context->stream.get();
    int64_t current_pos = stream->tell();
    if (current_pos < 0)
    {
        return -1;
    }
    int64_t size = stream->seek(0, core::stream::SeekMode::End);
    stream->seek(current_pos, core::stream::SeekMode::Begin);
    return size;
}

Sint64 SDLCALL cyanvne::resources::adapters::sdl_seek_cb(void* userdata, Sint64 offset, SDL_IOWhence whence)
{
    auto* context = static_cast<SdlAdapterContext*>(userdata);
    if (!context || !context->stream)
    {
        return -1;
    }
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

size_t cyanvne::resources::adapters::sdl_read_cb(void* userdata, void* ptr, size_t size, SDL_IOStatus* status)
{
	auto* context = static_cast<SdlAdapterContext*>(userdata);
	if (!context || !context->stream)
	{
		if (status)
		{
			*status = SDL_IO_STATUS_ERROR;
		}
		return 0;
	}
	size_t bytes_read = context->stream->read(ptr, size);
	if (bytes_read < size)
	{
		if (context->stream->tell() >= sdl_size_cb(userdata))
		{
			if (status)
			{
				*status = SDL_IO_STATUS_EOF;
			}
		}
		else
		{
			if (status)
			{
				*status = SDL_IO_STATUS_ERROR;
			}
		}
	}
	return bytes_read;
}

size_t cyanvne::resources::adapters::sdl_write_cb(void* userdata, const void* ptr, size_t size, SDL_IOStatus* status)
{
	if (status)
	{
		*status = SDL_IO_STATUS_ERROR;
	}
	SDL_SetError("This stream is read-only");
	return 0;
}

bool cyanvne::resources::adapters::sdl_close_cb(void* userdata)
{
	if (userdata)
	{
		delete static_cast<SdlAdapterContext*>(userdata);
	}
	return true;
}

SDL_IOStream* cyanvne::resources::adapters::CreateSdlIoStreamFromCyanvneStream(
	std::shared_ptr<core::stream::InStreamInterface> cyanvne_stream)
{
	if (!cyanvne_stream || !cyanvne_stream->is_open())
	{
		return nullptr;
	}

	SdlAdapterContext* context = new(std::nothrow) SdlAdapterContext{ std::move(cyanvne_stream) };
	if (!context)
	{
		return nullptr;
	}

	SDL_IOStreamInterface iface;
	SDL_INIT_INTERFACE(&iface);

	iface.size = sdl_size_cb;
	iface.seek = sdl_seek_cb;
	iface.read = sdl_read_cb;
	iface.write = sdl_write_cb;
	iface.close = sdl_close_cb;

	return SDL_OpenIO(&iface, context);
}
