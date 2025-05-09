#pragma once
#include <cstdint>

namespace cyanvne
{
	namespace core
	{
		namespace stream
		{
			enum SeekMode
			{
				Begin,
				Current,
				End
			};

			class InStreamInterface
			{
			protected:
				InStreamInterface() = default;
			public:
                InStreamInterface(const InStreamInterface&) = delete;
				InStreamInterface& operator=(const InStreamInterface&) = delete;
				InStreamInterface(InStreamInterface&&) = delete;
                InStreamInterface& operator=(InStreamInterface&&) = delete;

                virtual bool read(void* buffer, size_t size) = 0;
                virtual bool seek(int64_t offset, SeekMode mode) = 0;
                virtual int64_t tell() = 0;
				virtual bool is_open() = 0;

                virtual ~InStreamInterface() = default;
			};

			class OutStreamInterface
			{
			protected:
				OutStreamInterface() = default;
			public:
				OutStreamInterface(const OutStreamInterface&) = delete;
				OutStreamInterface& operator=(const OutStreamInterface&) = delete;
				OutStreamInterface(OutStreamInterface&&) = delete;
				OutStreamInterface& operator=(OutStreamInterface&&) = delete;

				virtual bool write(const void* buffer, size_t size) = 0;
				virtual bool seek(int64_t offset, SeekMode mode) = 0;
				virtual int64_t tell() = 0;
				virtual void flush() = 0;
				virtual bool is_open() = 0;

				virtual ~OutStreamInterface() = default;
			};

			class StreamInterface : public InStreamInterface, public OutStreamInterface
			{
			protected:
				StreamInterface() = default;
			public:
				StreamInterface(const StreamInterface&) = delete;
				StreamInterface& operator=(const StreamInterface&) = delete;
				StreamInterface(StreamInterface&&) = delete;
				StreamInterface& operator=(StreamInterface&&) = delete;

                ~StreamInterface() override = default;
			};
		}
	}
}