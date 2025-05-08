#pragma once

namespace cyanvne
{
	namespace core
	{
		namespace stream
		{
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
                virtual bool seek(size_t offset) = 0;
                virtual size_t tell() = 0;
				virtual bool isReadable() = 0;

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
                virtual bool seek(size_t offset) = 0;
                virtual size_t tell() = 0;
                virtual void flush() = 0;
                virtual bool isWritable() = 0;

                virtual ~OutStreamInterface() = default;
			}
		}
	}
}