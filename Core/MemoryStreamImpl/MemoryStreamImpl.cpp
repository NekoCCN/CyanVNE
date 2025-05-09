#include "MemoryStreamImpl.h"

cyanvne::core::stream::DynamicMemoryStreamImpl::DynamicMemoryStreamImpl(const void* initial_data, size_t initial_size) : position_(0)
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

cyanvne::core::stream::DynamicMemoryStreamImpl::DynamicMemoryStreamImpl(size_t initial_capacity) : position_(0)
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

cyanvne::core::stream::DynamicMemoryStreamImpl::DynamicMemoryStreamImpl(const DynamicMemoryStreamImpl& other)
{
	position_ = other.position_;
	data_ = other.data_;
}

cyanvne::core::stream::DynamicMemoryStreamImpl::DynamicMemoryStreamImpl(DynamicMemoryStreamImpl&& other) noexcept
{
	position_ = other.position_;
	data_ = std::move(other.data_);
}

cyanvne::core::stream::DynamicMemoryStreamImpl& cyanvne::core::stream::DynamicMemoryStreamImpl::operator=(
	const DynamicMemoryStreamImpl& other)
{
	position_ = other.position_;
	data_ = other.data_;
	return *this;
}

cyanvne::core::stream::DynamicMemoryStreamImpl& cyanvne::core::stream::DynamicMemoryStreamImpl::operator=(
	DynamicMemoryStreamImpl&& other) noexcept
{
	position_ = other.position_;
	data_ = std::move(other.data_);
	return *this;
}

bool cyanvne::core::stream::DynamicMemoryStreamImpl::read(void* buffer, size_t size)
{
    std::lock_guard<std::mutex> lock(mutex_);

    if (buffer == nullptr && size > 0)
    {
        throw exception::NullPointerException("DynamicMemoryStreamImpl: Null pointer in read for non-zero size");
    }
    if (size == 0)
    {
        return true;
    }

    if (position_ >= data_.size() || size > (data_.size() - position_))
    {
        return false;
    }

    memcpy(buffer, data_.data() + position_, size);
    position_ += size;
    return true;
}

bool cyanvne::core::stream::DynamicMemoryStreamImpl::write(const void* buffer, size_t size)
{
    std::lock_guard<std::mutex> lock(mutex_);

    if (buffer == nullptr && size > 0)
    {
        throw exception::NullPointerException("DynamicMemoryStreamImpl: Null pointer in write for non-zero size");
    }
    if (size == 0)
    {
        return true;
    }

    uint64_t required_size = position_ + size;
    try
    {
        if (required_size > data_.size())
        {
            data_.resize(required_size);
        }
    }
    catch (const std::bad_alloc&)
    {
        throw exception::MemoryAllocException("DynamicMemoryStreamImpl: Memory allocation failed in write");
    }
    catch (const std::length_error&)
    {
        throw exception::IllegalArgumentException("DynamicMemoryStreamImpl: Requested size too large in write");
    }

    memcpy(data_.data() + position_, buffer, size);
    position_ += size;
    return true;
}

bool cyanvne::core::stream::DynamicMemoryStreamImpl::seek(int64_t offset, SeekMode mode)
{
    std::lock_guard<std::mutex> lock(mutex_);
    uint64_t new_pos_val;
    uint64_t current_data_size = data_.size();

    switch (mode)
    {
    case SeekMode::Begin:
    {
        if (offset < 0)
        {
            return false;
        }
        new_pos_val = static_cast<uint64_t>(offset);
        break;
    }
    case SeekMode::Current:
    {
        int64_t current_pos_signed = static_cast<int64_t>(position_);
        int64_t temp_new_pos = current_pos_signed + offset;
        if (temp_new_pos < 0)
        {
            return false;
        }
        new_pos_val = static_cast<uint64_t>(temp_new_pos);
        break;
    }
    case SeekMode::End:
    {
        int64_t data_size_signed = static_cast<int64_t>(current_data_size);
        int64_t temp_new_pos = data_size_signed + offset;
        if (temp_new_pos < 0)
        {
            return false;
        }
        new_pos_val = static_cast<uint64_t>(temp_new_pos);
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

bool cyanvne::core::stream::DynamicMemoryStreamImpl::boundedSeek(int64_t offset, SeekMode mode)
{
    std::lock_guard<std::mutex> lock(mutex_);
    uint64_t new_pos_val;

    switch (mode)
    {
    case SeekMode::Begin:
    {
        if (offset < 0 || static_cast<uint64_t>(offset) > data_.size())
        {
            return false;
        }
        new_pos_val = static_cast<uint64_t>(offset);
        break;
    }
    case SeekMode::Current:
    {
        int64_t current_pos_signed = static_cast<int64_t>(position_);
        int64_t temp_new_pos = current_pos_signed + offset;
        if (temp_new_pos < 0 || static_cast<uint64_t>(temp_new_pos) > data_.size())
        {
            return false;
        }
        new_pos_val = static_cast<uint64_t>(temp_new_pos);
        break;
    }
    case SeekMode::End:
    {
        int64_t data_size_signed = static_cast<int64_t>(data_.size());
        int64_t temp_new_pos = data_size_signed + offset;
        if (temp_new_pos < 0 || static_cast<uint64_t>(temp_new_pos) > data_.size())
        {
            return false;
        }
        new_pos_val = static_cast<uint64_t>(temp_new_pos);
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

int64_t cyanvne::core::stream::DynamicMemoryStreamImpl::tell()
{
    std::lock_guard<std::mutex> lock(mutex_);
    return static_cast<int64_t>(position_);
}

bool cyanvne::core::stream::DynamicMemoryStreamImpl::is_open()
{
    return true;
}

void cyanvne::core::stream::DynamicMemoryStreamImpl::flush()
{

}

const std::vector<uint8_t>& cyanvne::core::stream::DynamicMemoryStreamImpl::get_data() const
{
    std::lock_guard<std::mutex> lock(mutex_);
    return data_;
}

std::vector<uint8_t> cyanvne::core::stream::DynamicMemoryStreamImpl::copyData() const
{
    std::lock_guard<std::mutex> lock(mutex_);
    return data_;
}

uint64_t cyanvne::core::stream::DynamicMemoryStreamImpl::getSize() const
{
    std::lock_guard<std::mutex> lock(mutex_);
    return data_.size();
}

void cyanvne::core::stream::DynamicMemoryStreamImpl::clear()
{
    std::lock_guard<std::mutex> lock(mutex_);
    data_.clear();
    position_ = 0;
}

cyanvne::core::stream::FixedSizeMemoryStreamImpl::FixedSizeMemoryStreamImpl(uint64_t size) : position_(0)
{
    try
    {
        data_.resize(size);
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

cyanvne::core::stream::FixedSizeMemoryStreamImpl::FixedSizeMemoryStreamImpl(const void* initial_data, size_t initial_size) : position_(0)
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

cyanvne::core::stream::FixedSizeMemoryStreamImpl::FixedSizeMemoryStreamImpl(const FixedSizeMemoryStreamImpl& other)
{
	position_ = other.position_;
	data_ = other.data_;
}

cyanvne::core::stream::FixedSizeMemoryStreamImpl::FixedSizeMemoryStreamImpl(FixedSizeMemoryStreamImpl&& other) noexcept
{
	position_ = other.position_;
	data_ = std::move(other.data_);
}

cyanvne::core::stream::FixedSizeMemoryStreamImpl& cyanvne::core::stream::FixedSizeMemoryStreamImpl::operator=(
	const FixedSizeMemoryStreamImpl& other)
{
	position_ = other.position_;
	data_ = other.data_;
	return *this;
}

cyanvne::core::stream::FixedSizeMemoryStreamImpl& cyanvne::core::stream::FixedSizeMemoryStreamImpl::operator=(
	FixedSizeMemoryStreamImpl&& other) noexcept
{
	position_ = other.position_;
	data_ = std::move(other.data_);
	return *this;
}

bool cyanvne::core::stream::FixedSizeMemoryStreamImpl::read(void* buffer, size_t size)
{
    std::lock_guard<std::mutex> lock(mutex_);

    if (buffer == nullptr && size > 0)
    {
        throw exception::NullPointerException("FixedSizeMemoryStreamImpl: Null pointer in read for non-zero size");
    }
    if (size == 0)
    {
        return true;
    }

    if (position_ >= data_.size() || size > (data_.size() - position_))
    {
        return false;
    }

    memcpy(buffer, data_.data() + position_, size);
    position_ += size;
    return true;
}

bool cyanvne::core::stream::FixedSizeMemoryStreamImpl::write(const void* buffer, size_t size)
{
    std::lock_guard<std::mutex> lock(mutex_);

    if (buffer == nullptr && size > 0)
    {
        throw exception::NullPointerException("FixedSizeMemoryStreamImpl: Null pointer in write for non-zero size");
    }
    if (size == 0)
    {
        return true;
    }

    if (position_ >= data_.size() || size > (data_.size() - position_))
    {
        return false;
    }

    memcpy(data_.data() + position_, buffer, size);
    position_ += size;
    return true;
}

bool cyanvne::core::stream::FixedSizeMemoryStreamImpl::seek(int64_t offset, SeekMode mode)
{
    std::lock_guard<std::mutex> lock(mutex_);
    uint64_t new_pos_val;

    switch (mode)
    {
    case SeekMode::Begin:
    {
        if (offset < 0 || static_cast<uint64_t>(offset) > data_.size())
        {
            return false;
        }
        new_pos_val = static_cast<uint64_t>(offset);
        break;
    }
    case SeekMode::Current:
    {
        int64_t current_pos_signed = static_cast<int64_t>(position_);
        int64_t temp_new_pos = current_pos_signed + offset;
        if (temp_new_pos < 0 || static_cast<uint64_t>(temp_new_pos) > data_.size())
        {
            return false;
        }
        new_pos_val = static_cast<uint64_t>(temp_new_pos);
        break;
    }
    case SeekMode::End:
    {
        int64_t data_size_signed = static_cast<int64_t>(data_.size());
        int64_t temp_new_pos = data_size_signed + offset;
        if (temp_new_pos < 0 || static_cast<uint64_t>(temp_new_pos) > data_.size())
        {
            return false;
        }
        new_pos_val = static_cast<uint64_t>(temp_new_pos);
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

int64_t cyanvne::core::stream::FixedSizeMemoryStreamImpl::tell()
{
    std::lock_guard<std::mutex> lock(mutex_);
    return static_cast<int64_t>(position_);
}

bool cyanvne::core::stream::FixedSizeMemoryStreamImpl::is_open()
{
    return true;
}

void cyanvne::core::stream::FixedSizeMemoryStreamImpl::flush()
{  }

uint64_t cyanvne::core::stream::FixedSizeMemoryStreamImpl::getCapacity() const
{
    std::lock_guard<std::mutex> lock(mutex_);
    return data_.size();
}
