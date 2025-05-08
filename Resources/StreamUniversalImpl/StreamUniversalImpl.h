#pragma once
#include <SDL3/SDL.h>
#include <Core/Logger/Logger.h>
#include <Core/Stream/Stream.h>
#include <cstdint>

namespace cyanvne
{
	namespace resources
	{
		class InStreamUniversalImpl : public core::stream::InStreamInterface
		{
		private:
			SDL_IOStream* in_stream_;
		protected:
			InStreamUniversalImpl(SDL_IOStream* in_stream)
				: in_stream_(in_stream)
			{  }
		public:
			InStreamUniversalImpl(const InStreamUniversalImpl& other) = delete;
            InStreamUniversalImpl(InStreamUniversalImpl&& other) = delete;
            InStreamUniversalImpl& operator=(const InStreamUniversalImpl& other) = delete;
            InStreamUniversalImpl& operator=(InStreamUniversalImpl&& other) = delete;

			static std::shared_ptr<InStreamUniversalImpl> createFromBinaryFile(const std::string& path)
			{
				core::GlobalLogger::getCoreLogger()->info("Try creating InStreamUniversalImpl from file");
				return std::make_shared<InStreamUniversalImpl>(SDL_IOFromFile(path.c_str(), "rb"));
			}
			static std::shared_ptr<InStreamUniversalImpl> createFromMemory(void* data, size_t size)
			{
				core::GlobalLogger::getCoreLogger()->info("Try creating InStreamUniversalImpl from memory", size);
				return std::make_shared<InStreamUniversalImpl>(SDL_IOFromMem(data, size));
			}

			bool read(void* buffer, size_t size) override
			{
				if (!in_stream_)
				{
					return false;
				}

				return SDL_ReadIO(in_stream_, buffer, size) == size;
			}
            bool seek(int64_t offset, core::stream::SeekMode mode) override
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
			int64_t tell() override
			{
				if (!in_stream_)
				{
					return -1;
				}

				return SDL_TellIO(in_stream_);
			}
            bool is_open() override
			{
				if (!in_stream_)
				{
					return false;
				}

				return SDL_GetIOStatus(in_stream_) == SDL_IO_STATUS_READY;
			}

            ~InStreamUniversalImpl() override
			{
				if (in_stream_)
				{
					SDL_CloseIO(in_stream_);
				}
			}
		};

		class OutStreamUniversalImpl : public core::stream::OutStreamInterface
		{
		private:
			SDL_IOStream* out_stream_;
		protected:
			OutStreamUniversalImpl(SDL_IOStream* out_stream)
				: out_stream_(out_stream)
			{  }
		public:
			OutStreamUniversalImpl(const OutStreamUniversalImpl& other) = delete;
            OutStreamUniversalImpl(OutStreamUniversalImpl&& other) = delete;
            OutStreamUniversalImpl& operator=(const OutStreamUniversalImpl& other) = delete;
            OutStreamUniversalImpl& operator=(OutStreamUniversalImpl&& other) = delete;

			static std::shared_ptr<OutStreamUniversalImpl> createFromBinaryFile(const std::string& path)
			{
				core::GlobalLogger::getCoreLogger()->info("Try creating OutStreamUniversalImpl from file");
				return std::make_shared<OutStreamUniversalImpl>(SDL_IOFromFile(path.c_str(), "wb"));
			}
			static std::shared_ptr<OutStreamUniversalImpl> createFromMemory(void* data, size_t size)
			{
				core::GlobalLogger::getCoreLogger()->info("Try creating OutStreamUniversalImpl from memory");
				return std::make_shared<OutStreamUniversalImpl>(SDL_IOFromMem(data, size));
			}

			bool write(const void* buffer, size_t size) override
			{
				if (!out_stream_)
				{
					return false;
				}

				return SDL_WriteIO(out_stream_, buffer, size) == size;
			}
			bool seek(int64_t offset, core::stream::SeekMode mode) override
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
			int64_t tell() override
			{
				if (!out_stream_)
				{
					return -1;
				}

				return SDL_TellIO(out_stream_);
			}
			void flush() override
			{
				if (!out_stream_)
				{
					return;
				}

				SDL_FlushIO(out_stream_);
			}
			bool is_open() override
			{
				if (!out_stream_)
				{
					return false;
				}

				return SDL_GetIOStatus(out_stream_) == SDL_IO_STATUS_READY;
			}

			~OutStreamUniversalImpl() override
			{
				if (out_stream_)
				{
					SDL_CloseIO(out_stream_);
				}
			}
		};

		class DynamicMemoryStreamImpl : public core::stream::InStreamInterface, core::stream::OutStreamInterface
		{
		private:
			SDL_IOStream* stream_;
		public:
			DynamicMemoryStreamImpl()
			{
				stream_ = SDL_IOFromDynamicMem();
			}

            DynamicMemoryStreamImpl(const DynamicMemoryStreamImpl&) = delete;
            DynamicMemoryStreamImpl(DynamicMemoryStreamImpl&&) = delete;
            DynamicMemoryStreamImpl& operator=(const DynamicMemoryStreamImpl&) = delete;
            DynamicMemoryStreamImpl& operator=(DynamicMemoryStreamImpl&&) = delete;

			bool read(void* buffer, size_t size) override
			{
				if (!stream_)
				{
					return false;
				}

				return SDL_ReadIO(stream_, buffer, size) == size;
			}
			bool write(const void* buffer, size_t size) override
			{
				if (!stream_)
				{
					return false;
				}

				return SDL_WriteIO(stream_, buffer, size) == size;
			}
			bool seek(int64_t offset, core::stream::SeekMode mode) override
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
            int64_t tell() override
			{
				if (!stream_)
				{
					return -1;
				}

				return SDL_TellIO(stream_);
			}
			void flush() override
			{
				if (!stream_)
				{
					return;
				}

                SDL_FlushIO(stream_);
			}
			bool is_open() override
			{
				if (!stream_)
				{
					return false;
				}

				return SDL_GetIOStatus(stream_) == SDL_IO_STATUS_READY;
			}

            ~DynamicMemoryStreamImpl() override
			{
				if (stream_)
				{
					SDL_CloseIO(stream_);
				}
			}
		};
	}
}