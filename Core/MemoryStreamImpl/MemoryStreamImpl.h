#pragma once
#include <vector>
#include <memory>
#include <mutex>
#include <cstdint>
#include <cstring>
#include <algorithm>
#include <Core/CoreException/CoreException.h>
#include <Core/Stream/Stream.h>

namespace cyanvne
{
    namespace core
    {
        namespace stream
        {
            class DynamicMemoryStreamImpl : public StreamInterface
            {
            private:
                mutable std::mutex mutex_;

                std::vector<uint8_t> data_;
                uint64_t position_;
            public:
                DynamicMemoryStreamImpl() : position_(0)
                {  }
                DynamicMemoryStreamImpl(const void* initial_data, size_t initial_size);
                explicit DynamicMemoryStreamImpl(size_t initial_capacity);

                DynamicMemoryStreamImpl(const DynamicMemoryStreamImpl& other);
                DynamicMemoryStreamImpl(DynamicMemoryStreamImpl&& other) noexcept;
                DynamicMemoryStreamImpl& operator=(const DynamicMemoryStreamImpl& other);
                DynamicMemoryStreamImpl& operator=(DynamicMemoryStreamImpl&& other) noexcept;

                bool read(void* buffer, size_t size) override;
                bool write(const void* buffer, size_t size) override;
                bool seek(int64_t offset, SeekMode mode) override;
                bool boundedSeek(int64_t offset, SeekMode mode);
                int64_t tell() override;
                bool is_open() override;
                void flush() override;
                const std::vector<uint8_t>& get_data() const;
                std::vector<uint8_t> copyData() const;
                uint64_t getSize() const;
                void clear();

                ~DynamicMemoryStreamImpl() override = default;
            };

            class FixedSizeMemoryStreamImpl : public StreamInterface
            {
            private:
                mutable std::mutex mutex_;

                std::vector<uint8_t> data_;
                uint64_t position_;
            public:
                explicit FixedSizeMemoryStreamImpl(uint64_t size);
                FixedSizeMemoryStreamImpl(const void* initial_data, size_t initial_size);

                FixedSizeMemoryStreamImpl(const FixedSizeMemoryStreamImpl& other);
                FixedSizeMemoryStreamImpl(FixedSizeMemoryStreamImpl&& other) noexcept;
                FixedSizeMemoryStreamImpl& operator=(const FixedSizeMemoryStreamImpl& other);
                FixedSizeMemoryStreamImpl& operator=(FixedSizeMemoryStreamImpl&& other) noexcept;

                bool read(void* buffer, size_t size) override;
                bool write(const void* buffer, size_t size) override;
                bool seek(int64_t offset, SeekMode mode) override;
                int64_t tell() override;
                bool is_open() override;
                void flush() override;
                uint64_t getCapacity() const;

                ~FixedSizeMemoryStreamImpl() override = default;
            };
        }
    }
}