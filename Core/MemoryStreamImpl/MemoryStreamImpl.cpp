#include "MemoryStreamImpl.h"
#include <cstring>
#include <algorithm>
#include <Core/CoreException/CoreException.h>
#include <memory>


namespace cyanvne
{
    namespace core
    {
        namespace stream
        {

            DynamicMemoryStreamImpl::DynamicMemoryStreamImpl(const void* initial_data, size_t initial_size) : position_(0)
            {
                if (initial_data && initial_size > 0)
                {
                    try
                    {
                        data_.resize(initial_size);
                        memcpy(data_.data(), initial_data, initial_size);
                    }
                    catch (const std::bad_alloc&)
                    {
                        throw exception::MemoryAllocException("DynamicMemoryStreamImpl: Memory allocation failed in constructor");
                    }
                }
            }

            DynamicMemoryStreamImpl::DynamicMemoryStreamImpl(size_t initial_capacity) : position_(0)
            {
                try
                {
                    data_.reserve(initial_capacity);
                }
                catch (const std::bad_alloc&)
                {
                    throw exception::MemoryAllocException("DynamicMemoryStreamImpl: Memory allocation failed in constructor");
                }
            }

            DynamicMemoryStreamImpl::DynamicMemoryStreamImpl(const DynamicMemoryStreamImpl& other)
            {
                std::lock_guard<std::mutex> lock(other.mutex_);
                position_ = other.position_;
                data_ = other.data_;
            }

            DynamicMemoryStreamImpl::DynamicMemoryStreamImpl(DynamicMemoryStreamImpl&& other) noexcept
            {
                std::lock_guard<std::mutex> lock(other.mutex_);
                position_ = other.position_;
                data_ = std::move(other.data_);
                other.position_ = 0;
            }

            DynamicMemoryStreamImpl& DynamicMemoryStreamImpl::operator=(const DynamicMemoryStreamImpl& other)
            {
                if (this == &other)
                {
                    return *this;
                }
                std::unique_lock<std::mutex> lock_this(mutex_, std::defer_lock);
                std::unique_lock<std::mutex> lock_other(other.mutex_, std::defer_lock);
                std::lock(lock_this, lock_other);

                position_ = other.position_;
                data_ = other.data_;
                return *this;
            }

            DynamicMemoryStreamImpl& DynamicMemoryStreamImpl::operator=(DynamicMemoryStreamImpl&& other) noexcept
            {
                if (this == &other)
                {
                    return *this;
                }
                std::unique_lock<std::mutex> lock_this(mutex_, std::defer_lock);
                std::unique_lock<std::mutex> lock_other(other.mutex_, std::defer_lock);
                std::lock(lock_this, lock_other);

                position_ = other.position_;
                data_ = std::move(other.data_);
                other.position_ = 0;
                other.data_.clear();
                return *this;
            }

            size_t DynamicMemoryStreamImpl::read(void* buffer, size_t size)
            {
                std::lock_guard<std::mutex> lock(mutex_);

                if (buffer == nullptr && size > 0)
                {
                    throw exception::NullPointerException("DynamicMemoryStreamImpl: Null pointer in read for non-zero size");
                }
                if (size == 0)
                {
                    return 0;
                }

                if (position_ >= data_.size())
                {
                    return 0;
                }

                size_t bytes_to_read = std::min(size, static_cast<size_t>(data_.size() - position_));

                if (bytes_to_read > 0)
                {
                    memcpy(buffer, data_.data() + position_, bytes_to_read);
                    position_ += bytes_to_read;
                }
                return bytes_to_read;
            }

            size_t DynamicMemoryStreamImpl::write(const void* buffer, size_t size)
            {
                std::lock_guard<std::mutex> lock(mutex_);

                if (buffer == nullptr && size > 0)
                {
                    throw exception::NullPointerException("DynamicMemoryStreamImpl: Null pointer in write for non-zero size");
                }
                if (size == 0)
                {
                    return 0;
                }

                uint64_t required_total_size = position_ + size;
                if (size > 0 && position_ > UINT64_MAX - size)
                {
                    throw exception::IllegalArgumentException("DynamicMemoryStreamImpl: Requested write results in size overflow");
                }

                try
                {
                    if (required_total_size > data_.size())
                    {
                        data_.resize(required_total_size);
                    }
                }
                catch (const std::bad_alloc&)
                {
                    throw exception::MemoryAllocException("DynamicMemoryStreamImpl: Memory allocation failed in write");
                }
                catch (const std::length_error&)
                {
                    throw exception::IllegalArgumentException("DynamicMemoryStreamImpl: Requested size too large in write (vector length_error)");
                }

                memcpy(data_.data() + position_, buffer, size);
                position_ += size;
                return size;
            }

            int64_t DynamicMemoryStreamImpl::seek(int64_t offset, SeekMode mode)
            {
                std::lock_guard<std::mutex> lock(mutex_);
                uint64_t new_calculated_pos_u64;

                switch (mode)
                {
                case SeekMode::Begin:
                {
                    if (offset < 0)
                    {
                        return -1;
                    }
                    new_calculated_pos_u64 = static_cast<uint64_t>(offset);
                    break;
                }
                case SeekMode::Current:
                {
                    if (offset >= 0)
                    {
                        if (position_ > UINT64_MAX - static_cast<uint64_t>(offset))
                        {
                            return -1;
                        }
                        new_calculated_pos_u64 = position_ + static_cast<uint64_t>(offset);
                    }
                    else
                    {
                        uint64_t abs_offset = static_cast<uint64_t>(-offset);
                        if (abs_offset > position_)
                        {
                            return -1;
                        }
                        new_calculated_pos_u64 = position_ - abs_offset;
                    }
                    break;
                }
                case SeekMode::End:
                {
                    uint64_t current_data_size = data_.size();
                    if (offset >= 0)
                    {
                        if (current_data_size > UINT64_MAX - static_cast<uint64_t>(offset))
                        {
                            return -1;
                        }
                        new_calculated_pos_u64 = current_data_size + static_cast<uint64_t>(offset);
                    }
                    else
                    {
                        uint64_t abs_offset = static_cast<uint64_t>(-offset);
                        if (abs_offset > current_data_size) {
                            return -1;
                        }
                        new_calculated_pos_u64 = current_data_size - abs_offset;
                    }
                    break;
                }
                default:
                    return -1;
                }

                position_ = new_calculated_pos_u64;

                return static_cast<int64_t>(position_);
            }

            bool DynamicMemoryStreamImpl::boundedSeek(int64_t offset, SeekMode mode)
            {
                std::lock_guard<std::mutex> lock(mutex_);
                uint64_t new_pos_val;
                uint64_t current_data_size = data_.size();

                switch (mode)
                {
                case SeekMode::Begin:
                {
                    if (offset < 0 || static_cast<uint64_t>(offset) > current_data_size)
                    {
                        return false;
                    }
                    new_pos_val = static_cast<uint64_t>(offset);
                    break;
                }
                case SeekMode::Current:
                {
                    uint64_t target_pos_u64;
                    if (offset >= 0)
                    {
                        if (position_ > UINT64_MAX - static_cast<uint64_t>(offset))
                        {
                            return false;
                        }
                        target_pos_u64 = position_ + static_cast<uint64_t>(offset);
                    }
                    else
                    {
                        if (static_cast<uint64_t>(-offset) > position_)
                        {
                            return false;
                        }
                        target_pos_u64 = position_ - static_cast<uint64_t>(-offset);
                    }

                    if (target_pos_u64 > current_data_size)
                    {
                        return false;
                    }
                    new_pos_val = target_pos_u64;
                    break;
                }
                case SeekMode::End:
                {
                    uint64_t target_pos_u64;
                    if (offset >= 0)
                    {
                        if (current_data_size > UINT64_MAX - static_cast<uint64_t>(offset))
                        {
                            return false;
                        }
                        target_pos_u64 = current_data_size + static_cast<uint64_t>(offset);
                    }
                    else
                    {
                        if (static_cast<uint64_t>(-offset) > current_data_size)
                        {
                            return false;
                        }
                        target_pos_u64 = current_data_size - static_cast<uint64_t>(-offset);
                    }

                    if (target_pos_u64 > current_data_size)
                    {
                        return false;
                    }
                    new_pos_val = target_pos_u64;
                    break;
                }
                default:
                {
                    return false;
                }
                }
                position_ = new_pos_val;
                return true;
            }


            int64_t DynamicMemoryStreamImpl::tell()
            {
                std::lock_guard<std::mutex> lock(mutex_);
                return static_cast<int64_t>(position_);
            }

            bool DynamicMemoryStreamImpl::is_open()
            {
                return true;
            }

            void DynamicMemoryStreamImpl::flush()
            {
                std::lock_guard<std::mutex> lock(mutex_);
            }

            const std::vector<uint8_t>& DynamicMemoryStreamImpl::get_data() const
            {
                std::lock_guard<std::mutex> lock(mutex_);
                return data_;
            }

            std::vector<uint8_t> DynamicMemoryStreamImpl::copyData() const
            {
                std::lock_guard<std::mutex> lock(mutex_);
                return data_;
            }

            uint64_t DynamicMemoryStreamImpl::getSize() const
            {
                std::lock_guard<std::mutex> lock(mutex_);
                return data_.size();
            }

            void DynamicMemoryStreamImpl::clear()
            {
                std::lock_guard<std::mutex> lock(mutex_);
                data_.clear();
                position_ = 0;
            }

            FixedSizeMemoryStreamImpl::FixedSizeMemoryStreamImpl(uint64_t size) : position_(0)
            {
                try
                {
                    data_.resize(static_cast<size_t>(size));
                }
                catch (const std::bad_alloc&)
                {
                    throw exception::MemoryAllocException("FixedSizeMemoryStreamImpl: Memory allocation failed in constructor");
                }
                catch (const std::length_error&)
                {
                    throw exception::MemoryAllocException("FixedSizeMemoryStreamImpl: Requested size too large in constructor");
                }
            }

            FixedSizeMemoryStreamImpl::FixedSizeMemoryStreamImpl(const void* initial_data, size_t initial_size) : position_(0)
            {
                if (initial_data == nullptr && initial_size > 0)
                {
                    throw exception::NullPointerException("FixedSizeMemoryStreamImpl: Null initial_data with non-zero size");
                }
                if (initial_size == 0)
                {
                    return;
                }
                try
                {
                    data_.resize(initial_size);
                    memcpy(data_.data(), initial_data, initial_size);
                }
                catch (const std::bad_alloc&)
                {
                    throw exception::MemoryAllocException("FixedSizeMemoryStreamImpl: Memory allocation failed in constructor");
                }
                catch (const std::length_error&)
                {
                    throw exception::IllegalArgumentException("FixedSizeMemoryStreamImpl: Requested size too large in constructor");
                }
            }

            FixedSizeMemoryStreamImpl::FixedSizeMemoryStreamImpl(const FixedSizeMemoryStreamImpl& other)
            {
                std::lock_guard<std::mutex> lock(other.mutex_);
                position_ = other.position_;
                data_ = other.data_;
            }

            FixedSizeMemoryStreamImpl::FixedSizeMemoryStreamImpl(FixedSizeMemoryStreamImpl&& other) noexcept
            {
                std::lock_guard<std::mutex> lock(other.mutex_);
                position_ = other.position_;
                data_ = std::move(other.data_);
                other.position_ = 0;
            }

            FixedSizeMemoryStreamImpl& FixedSizeMemoryStreamImpl::operator=(const FixedSizeMemoryStreamImpl& other)
            {
                if (this == &other)
                {
                    return *this;
                }
                std::unique_lock<std::mutex> lock_this(mutex_, std::defer_lock);
                std::unique_lock<std::mutex> lock_other(other.mutex_, std::defer_lock);
                std::lock(lock_this, lock_other);

                position_ = other.position_;
                data_ = other.data_;
                return *this;
            }

            FixedSizeMemoryStreamImpl& FixedSizeMemoryStreamImpl::operator=(FixedSizeMemoryStreamImpl&& other) noexcept
            {
                if (this == &other)
                {
                    return *this;
                }
                std::unique_lock<std::mutex> lock_this(mutex_, std::defer_lock);
                std::unique_lock<std::mutex> lock_other(other.mutex_, std::defer_lock);
                std::lock(lock_this, lock_other);

                position_ = other.position_;
                data_ = std::move(other.data_);
                other.position_ = 0;
                other.data_.clear();
                return *this;
            }


            size_t FixedSizeMemoryStreamImpl::read(void* buffer, size_t size)
            {
                std::lock_guard<std::mutex> lock(mutex_);

                if (buffer == nullptr && size > 0)
                {
                    throw exception::NullPointerException("FixedSizeMemoryStreamImpl: Null pointer in read for non-zero size");
                }
                if (size == 0)
                {
                    return 0;
                }

                if (position_ >= data_.size())
                {
                    return 0;
                }

                size_t bytes_readable = static_cast<size_t>(data_.size() - position_);
                size_t bytes_to_read = std::min(size, bytes_readable);

                if (bytes_to_read > 0) {
                    memcpy(buffer, data_.data() + position_, bytes_to_read);
                    position_ += bytes_to_read;
                }
                return bytes_to_read;
            }

            size_t FixedSizeMemoryStreamImpl::write(const void* buffer, size_t size)
            {
                std::lock_guard<std::mutex> lock(mutex_);

                if (buffer == nullptr && size > 0)
                {
                    throw exception::NullPointerException("FixedSizeMemoryStreamImpl: Null pointer in write for non-zero size");
                }
                if (size == 0)
                {
                    return 0;
                }

                if (position_ >= data_.size())
                {
                    return 0;
                }

                size_t space_available = static_cast<size_t>(data_.size() - position_);
                size_t bytes_to_write = std::min(size, space_available);

                if (bytes_to_write > 0)
                {
                    memcpy(data_.data() + position_, buffer, bytes_to_write);
                    position_ += bytes_to_write;
                }
                return bytes_to_write;
            }

            int64_t FixedSizeMemoryStreamImpl::seek(int64_t offset, SeekMode mode)
            {
                std::lock_guard<std::mutex> lock(mutex_);
                uint64_t new_calculated_pos_u64;
                uint64_t stream_capacity = data_.size();

                switch (mode)
                {
                case SeekMode::Begin:
                {
                    if (offset < 0)
                    {
                        return -1;
                    }
                    new_calculated_pos_u64 = static_cast<uint64_t>(offset);
                    break;
                }
                case SeekMode::Current:
                {
                    if (offset >= 0)
                    {
                        if (position_ > UINT64_MAX - static_cast<uint64_t>(offset)) {
                            return -1;
                        }
                        new_calculated_pos_u64 = position_ + static_cast<uint64_t>(offset);
                    }
                    else
                    {
                        uint64_t abs_offset = static_cast<uint64_t>(-offset);
                        if (abs_offset > position_)
                        {
                            return -1;
                        }
                        new_calculated_pos_u64 = position_ - abs_offset;
                    }
                    break;
                }
                case SeekMode::End:
                {
                    if (offset >= 0)
                    {
                        if (stream_capacity > UINT64_MAX - static_cast<uint64_t>(offset))
                        {
                            return -1;
                        }
                        new_calculated_pos_u64 = stream_capacity + static_cast<uint64_t>(offset);
                    }
                    else
                    {
                        uint64_t abs_offset = static_cast<uint64_t>(-offset);
                        if (abs_offset > stream_capacity)
                        {
                            return -1;
                        }
                        new_calculated_pos_u64 = stream_capacity - abs_offset;
                    }
                    break;
                }
                default:
                    return -1;
                }

                if (new_calculated_pos_u64 > stream_capacity)
                {
                    return -1;
                }

                position_ = new_calculated_pos_u64;

                return static_cast<int64_t>(position_);
            }

            int64_t FixedSizeMemoryStreamImpl::tell()
            {
                std::lock_guard<std::mutex> lock(mutex_);
                return static_cast<int64_t>(position_);
            }

            bool FixedSizeMemoryStreamImpl::is_open()
            {
                return true;
            }

            void FixedSizeMemoryStreamImpl::flush()
            {
                std::lock_guard<std::mutex> lock(mutex_);
            }

            uint64_t FixedSizeMemoryStreamImpl::getCapacity() const
            {
                std::lock_guard<std::mutex> lock(mutex_);
                return data_.size();
            }

        }
    }
}