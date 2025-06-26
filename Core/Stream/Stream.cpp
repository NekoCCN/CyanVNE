#include "Stream.h"
#include <vector>

uint64_t cyanvne::core::stream::utils::copy_stream_chunked(cyanvne::core::stream::InStreamInterface& in,
                                                           cyanvne::core::stream::OutStreamInterface& out, const size_t buffer_size)
{
	if (!in.is_open() || !out.is_open())
	{
		return 0;
	}

	if (buffer_size == 0)
	{
		return 0;
	}

	std::vector<char> buffer(buffer_size);
	uint64_t total_bytes_copied = 0;

	while (true)
	{
		size_t bytes_read_this_iteration = in.read(buffer.data(), buffer.size());

		if (bytes_read_this_iteration > 0)
		{
			size_t bytes_written_this_iteration = out.write(buffer.data(), bytes_read_this_iteration);
			if (bytes_written_this_iteration != bytes_read_this_iteration)
			{
				total_bytes_copied += bytes_written_this_iteration;
				break;
			}
			total_bytes_copied += bytes_written_this_iteration;
		}

		if (bytes_read_this_iteration < buffer.size())
		{
			break;
		}
	}
	return total_bytes_copied;
}

int64_t cyanvne::core::stream::utils::instream_size(cyanvne::core::stream::InStreamInterface& in)
{
	int64_t now = in.tell();
	if (now < 0)
	{
		return -1;
	}
	in.seek(0, cyanvne::core::stream::SeekMode::End);

	int64_t size = in.tell();

	in.seek(now, cyanvne::core::stream::SeekMode::Begin);

	return size;
}

int64_t cyanvne::core::stream::utils::outstream_size(cyanvne::core::stream::OutStreamInterface& out)
{
	int64_t now = out.tell();
	if (now < 0)
	{
		return -1;
	}
	out.seek(0, cyanvne::core::stream::SeekMode::End);

	int64_t size = out.tell();

	out.seek(now, cyanvne::core::stream::SeekMode::Begin);

	return size;
}

size_t cyanvne::core::stream::SubStream::read(void* buffer, size_t size_to_read)
{
	if (size_to_read == 0)
	{
		return 0;
	}

	const uint64_t remaining_bytes = resource_size_ - current_position_;
	if (remaining_bytes == 0)
	{
		return 0;
	}

	const size_t bytes_to_read = static_cast<size_t>(std::min(static_cast<uint64_t>(size_to_read), remaining_bytes));

	size_t bytes_actually_read = 0;
	{
		std::lock_guard<std::mutex> lock(parent_stream_mutex_);

		int64_t seek_pos = static_cast<int64_t>(resource_offset_ + current_position_);
		if (parent_stream_->seek(seek_pos, core::stream::SeekMode::Begin) != seek_pos)
		{
			return 0;
		}

		bytes_actually_read = parent_stream_->read(buffer, bytes_to_read);
	}

	if (bytes_actually_read > 0)
	{
		current_position_ += bytes_actually_read;
	}

	return bytes_actually_read;
}

int64_t cyanvne::core::stream::SubStream::seek(int64_t offset, core::stream::SeekMode mode)
{
	int64_t new_pos;
	switch (mode)
	{
	case core::stream::SeekMode::Begin:
		new_pos = offset;
		break;
	case core::stream::SeekMode::Current:
		new_pos = static_cast<int64_t>(current_position_) + offset;
		break;
	case core::stream::SeekMode::End:
		new_pos = static_cast<int64_t>(resource_size_) + offset;
		break;
	default:
		return -1;
	}

	if (new_pos < 0 || static_cast<uint64_t>(new_pos) > resource_size_)
	{
		return -1;
	}

	current_position_ = static_cast<uint64_t>(new_pos);
	return static_cast<int64_t>(current_position_);
}
