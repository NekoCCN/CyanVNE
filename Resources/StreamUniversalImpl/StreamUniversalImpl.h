#pragma once
#include <SDL3/SDL.h>
#include <Core/Logger/Logger.h>
#include <Core/Stream/Stream.h>
#include <Resources/ResourcesException/ResourcesException.h>
#include <cstdint>

namespace cyanvne
{
	namespace resources
	{
		class InStreamUniversalImpl : public core::stream::InStreamInterface
		{
		private:
			SDL_IOStream* in_stream_;
		public:
			InStreamUniversalImpl(SDL_IOStream* in_stream)
				: in_stream_(in_stream)
			{  }

			InStreamUniversalImpl(const InStreamUniversalImpl& other) = delete;
            InStreamUniversalImpl(InStreamUniversalImpl&& other) = delete;
            InStreamUniversalImpl& operator=(const InStreamUniversalImpl& other) = delete;
            InStreamUniversalImpl& operator=(InStreamUniversalImpl&& other) = delete;

			static std::shared_ptr<InStreamUniversalImpl> createFromBinaryFile(const std::string& path);
			static std::shared_ptr<InStreamUniversalImpl> createFromMemory(void* data, size_t size);

			size_t read(void* buffer, size_t size) override;
			int64_t seek(int64_t offset, core::stream::SeekMode mode) override;
			int64_t tell() override;
			bool is_open() override;

			~InStreamUniversalImpl() override;
		};

		class OutStreamUniversalImpl : public core::stream::OutStreamInterface
		{
		private:
			SDL_IOStream* out_stream_;
		public:
			OutStreamUniversalImpl(SDL_IOStream* out_stream)
				: out_stream_(out_stream)
			{  }

			OutStreamUniversalImpl(const OutStreamUniversalImpl& other) = delete;
            OutStreamUniversalImpl(OutStreamUniversalImpl&& other) = delete;
            OutStreamUniversalImpl& operator=(const OutStreamUniversalImpl& other) = delete;
            OutStreamUniversalImpl& operator=(OutStreamUniversalImpl&& other) = delete;

			static std::shared_ptr<OutStreamUniversalImpl> createFromBinaryFile(const std::string& path);
			static std::shared_ptr<OutStreamUniversalImpl> createFromMemory(void* data, size_t size);

			size_t write(const void* buffer, size_t size) override;
			int64_t seek(int64_t offset, core::stream::SeekMode mode) override;
			int64_t tell() override;
			void flush() override;
			bool is_open() override;

			~OutStreamUniversalImpl() override;
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

			size_t read(void* buffer, size_t size) override;
			size_t write(const void* buffer, size_t size) override;
			int64_t seek(int64_t offset, core::stream::SeekMode mode) override;
			int64_t tell() override;
			void flush() override;
			bool is_open() override;
			 
			~DynamicMemoryStreamImpl() override;
		};
	}
}