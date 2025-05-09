#include "StreamUniversalImpl.h"

std::shared_ptr<cyanvne::resources::InStreamUniversalImpl> cyanvne::resources::InStreamUniversalImpl::
createFromBinaryFile(const std::string& path)
{
	core::GlobalLogger::getCoreLogger()->info("Try creating InStreamUniversalImpl from file");
				
	SDL_IOStream* stream = SDL_IOFromFile(path.c_str(), "rb");
	if (!stream)
	{
		return nullptr;
	}

	return std::make_shared<InStreamUniversalImpl>(stream);
}

std::shared_ptr<cyanvne::resources::InStreamUniversalImpl> cyanvne::resources::InStreamUniversalImpl::createFromMemory(
	void* data, size_t size)
{
	core::GlobalLogger::getCoreLogger()->info("Try creating InStreamUniversalImpl from memory", size);
				
	SDL_IOStream* stream = SDL_IOFromMem(data, size);
	if (!stream)
	{
		return nullptr;
	}

	return std::make_shared<InStreamUniversalImpl>(stream);
}

bool cyanvne::resources::InStreamUniversalImpl::read(void* buffer, size_t size)
{
	if (!in_stream_)
	{
		return false;
	}
	if (buffer == nullptr)
	{
		throw exception::NullPointerException("InStreamUniversalImpl ：Null pointer in read");
	}

	return SDL_ReadIO(in_stream_, buffer, size) == size;
}

bool cyanvne::resources::InStreamUniversalImpl::seek(int64_t offset, core::stream::SeekMode mode)
{
	if (!in_stream_)
	{
		return false;
	}

	SDL_IOWhence whence = SDL_IO_SEEK_SET;
				
	switch (mode)
	{
	case core::stream::SeekMode::Begin:
		whence = SDL_IO_SEEK_SET;
		break;
	case core::stream::SeekMode::Current:
		whence = SDL_IO_SEEK_CUR;
		break;
	case core::stream::SeekMode::End:
		whence = SDL_IO_SEEK_END;
		break;
	}

	return SDL_SeekIO(in_stream_, offset, whence) >= 0;
}

int64_t cyanvne::resources::InStreamUniversalImpl::tell()
{
	if (!in_stream_)
	{
		return -1;
	}

	return SDL_TellIO(in_stream_);
}

bool cyanvne::resources::InStreamUniversalImpl::is_open()
{
	if (!in_stream_)
	{
		return false;
	}

	return SDL_GetIOStatus(in_stream_) == SDL_IO_STATUS_READY;
}

cyanvne::resources::InStreamUniversalImpl::~InStreamUniversalImpl()
{
	if (in_stream_)
	{
		SDL_CloseIO(in_stream_);
	}
}

std::shared_ptr<cyanvne::resources::OutStreamUniversalImpl> cyanvne::resources::OutStreamUniversalImpl::
createFromBinaryFile(const std::string& path)
{
	core::GlobalLogger::getCoreLogger()->info("Try creating OutStreamUniversalImpl from file");
	return std::make_shared<OutStreamUniversalImpl>(SDL_IOFromFile(path.c_str(), "wb"));
}

std::shared_ptr<cyanvne::resources::OutStreamUniversalImpl> cyanvne::resources::OutStreamUniversalImpl::
createFromMemory(void* data, size_t size)
{
	core::GlobalLogger::getCoreLogger()->info("Try creating OutStreamUniversalImpl from memory");
	return std::make_shared<OutStreamUniversalImpl>(SDL_IOFromMem(data, size));
}

bool cyanvne::resources::OutStreamUniversalImpl::write(const void* buffer, size_t size)
{
	if (!out_stream_)
	{
		return false;
	}
	if (buffer == nullptr)
	{
		throw exception::NullPointerException("InStreamUniversalImpl ：Null pointer in write");
	}

	return SDL_WriteIO(out_stream_, buffer, size) == size;
}

bool cyanvne::resources::OutStreamUniversalImpl::seek(int64_t offset, core::stream::SeekMode mode)
{
	if (!out_stream_)
	{
		return false;
	}

	SDL_IOWhence whence = SDL_IO_SEEK_SET;

	switch (mode)
	{
	case core::stream::SeekMode::Begin:
		whence = SDL_IO_SEEK_SET;
		break;
	case core::stream::SeekMode::Current:
		whence = SDL_IO_SEEK_CUR;
		break;
	case core::stream::SeekMode::End:
		whence = SDL_IO_SEEK_END;
		break;
	}

	return SDL_SeekIO(out_stream_, offset, whence) >= 0;
}

int64_t cyanvne::resources::OutStreamUniversalImpl::tell()
{
	if (!out_stream_)
	{
		return -1;
	}

	return SDL_TellIO(out_stream_);
}

void cyanvne::resources::OutStreamUniversalImpl::flush()
{
	if (!out_stream_)
	{
		return;
	}

	SDL_FlushIO(out_stream_);
}

bool cyanvne::resources::OutStreamUniversalImpl::is_open()
{
	if (!out_stream_)
	{
		return false;
	}

	return SDL_GetIOStatus(out_stream_) == SDL_IO_STATUS_READY;
}

cyanvne::resources::OutStreamUniversalImpl::~OutStreamUniversalImpl()
{
	if (out_stream_)
	{
		SDL_CloseIO(out_stream_);
	}
}

bool cyanvne::resources::DynamicMemoryStreamImpl::read(void* buffer, size_t size)
{
	if (!stream_)
	{
		return false;
	}

	return SDL_ReadIO(stream_, buffer, size) == size;
}

bool cyanvne::resources::DynamicMemoryStreamImpl::write(const void* buffer, size_t size)
{
	if (!stream_)
	{
		return false;
	}

	return SDL_WriteIO(stream_, buffer, size) == size;
}

bool cyanvne::resources::DynamicMemoryStreamImpl::seek(int64_t offset, core::stream::SeekMode mode)
{
	if (!stream_)
	{
		return false;
	}

	SDL_IOWhence whence = SDL_IO_SEEK_SET;

	switch (mode)
	{
	case core::stream::SeekMode::Begin:
		whence = SDL_IO_SEEK_SET;
		break;
	case core::stream::SeekMode::Current:
		whence = SDL_IO_SEEK_CUR;
		break;
	case core::stream::SeekMode::End:
		whence = SDL_IO_SEEK_END;
		break;
	}

	return SDL_SeekIO(stream_, offset, whence) >= 0;
}

int64_t cyanvne::resources::DynamicMemoryStreamImpl::tell()
{
	if (!stream_)
	{
		return -1;
	}

	return SDL_TellIO(stream_);
}

void cyanvne::resources::DynamicMemoryStreamImpl::flush()
{
	if (!stream_)
	{
		return;
	}

	SDL_FlushIO(stream_);
}

bool cyanvne::resources::DynamicMemoryStreamImpl::is_open()
{
	if (!stream_)
	{
		return false;
	}

	return SDL_GetIOStatus(stream_) == SDL_IO_STATUS_READY;
}

cyanvne::resources::DynamicMemoryStreamImpl::~DynamicMemoryStreamImpl()
{
	if (stream_)
	{
		SDL_CloseIO(stream_);
	}
}
