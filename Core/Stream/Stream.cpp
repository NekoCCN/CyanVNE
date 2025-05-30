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
