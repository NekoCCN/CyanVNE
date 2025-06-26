#pragma once
#include <cstdint>
#include <memory>
#include <mutex>

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

            class SubStream : public core::stream::InStreamInterface
            {
            private:
                std::shared_ptr<core::stream::InStreamInterface> parent_stream_;
                std::mutex& parent_stream_mutex_;

                uint64_t resource_offset_;
                uint64_t resource_size_;
                uint64_t current_position_;

            public:
                SubStream(std::shared_ptr<core::stream::InStreamInterface> parent, std::mutex& shared_mutex, uint64_t offset, uint64_t size)
                    : parent_stream_(std::move(parent)),
                    parent_stream_mutex_(shared_mutex),
                    resource_offset_(offset),
                    resource_size_(size),
                    current_position_(0)
                {  }

                SubStream(const SubStream&) = delete;
                SubStream& operator=(const SubStream&) = delete;
                SubStream(SubStream&&) = delete;
                SubStream& operator=(SubStream&&) = delete;
                ~SubStream() override = default;

                size_t read(void* buffer, size_t size_to_read) override;
                int64_t seek(int64_t offset, core::stream::SeekMode mode) override;

                int64_t tell() override
                {
                    return static_cast<int64_t>(current_position_);
                }

                bool is_open() override
                {
                    return parent_stream_ && parent_stream_->is_open();
                }

                uint64_t size() const
                {
                    return resource_size_;
                }
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
