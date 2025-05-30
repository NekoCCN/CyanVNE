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

			// Thread not safe
			class InStreamInterface
			{
			protected:
				InStreamInterface() = default;
			public:
                InStreamInterface(const InStreamInterface&) = delete;
				InStreamInterface& operator=(const InStreamInterface&) = delete;
				InStreamInterface(InStreamInterface&&) = delete;
                InStreamInterface& operator=(InStreamInterface&&) = delete;

                virtual size_t read(void* buffer, size_t size) = 0;
                virtual int64_t seek(int64_t offset, SeekMode mode) = 0;
                virtual int64_t tell() = 0;
				virtual bool is_open() = 0;

                virtual ~InStreamInterface() = default;
			};

			// Thread not safe
			class OutStreamInterface
			{
			protected:
				OutStreamInterface() = default;
			public:
				OutStreamInterface(const OutStreamInterface&) = delete;
				OutStreamInterface& operator=(const OutStreamInterface&) = delete;
				OutStreamInterface(OutStreamInterface&&) = delete;
				OutStreamInterface& operator=(OutStreamInterface&&) = delete;

				virtual size_t write(const void* buffer, size_t size) = 0;
				virtual int64_t seek(int64_t offset, SeekMode mode) = 0;
				virtual int64_t tell() = 0;
				virtual void flush() = 0;
				virtual bool is_open() = 0;

				virtual ~OutStreamInterface() = default;
			};

			// Thread not safe
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

			namespace utils
			{
				uint64_t copy_stream_chunked(
					InStreamInterface& in, OutStreamInterface& out, 
					size_t buffer_size = 8192);

				int64_t instream_size(InStreamInterface& in);
				int64_t outstream_size(OutStreamInterface& out);
			}
		}
	}
}
