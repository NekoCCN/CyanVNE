#include "ThemeConfig.h"

namespace cyanvne
{
	namespace parser
	{
		namespace theme
		{
			using namespace core::binaryserializer;
			using namespace core::stream;

			std::ptrdiff_t AnimationData::serialize(OutStreamInterface& out) const
			{
				std::ptrdiff_t total_bytes_written = 0;

				std::ptrdiff_t bytes_written = serialize_object(out, ms_per_frame);
				if (bytes_written == -1)
					return -1;
				total_bytes_written += bytes_written;

				bytes_written = serialize_object(out, sprite_sheet);
				if (bytes_written == -1)
					return -1;
				total_bytes_written += bytes_written;

				return total_bytes_written;
			}

			std::ptrdiff_t AnimationData::deserialize(InStreamInterface& in)
			{
				std::ptrdiff_t total_bytes_read = 0;

				std::ptrdiff_t bytes_read = deserialize_object(in, ms_per_frame);
				if (bytes_read == -1)
					return -1;
				total_bytes_read += bytes_read;

				bytes_read = deserialize_object(in, sprite_sheet);
				if (bytes_read == -1)
					return -1;
				total_bytes_read += bytes_read;

				return total_bytes_read;
			}

			std::ptrdiff_t AreaScaleData::serialize(OutStreamInterface& out) const
			{
				std::ptrdiff_t total_bytes_written = 0;

				std::ptrdiff_t bytes_written = serialize_object(out, x);
				if (bytes_written == -1)
					return -1;
				total_bytes_written += bytes_written;

				bytes_written = serialize_object(out, y);
				if (bytes_written == -1)
					return -1;
				total_bytes_written += bytes_written;

				bytes_written = serialize_object(out, w);
				if (bytes_written == -1)
					return -1;
				total_bytes_written += bytes_written;

				bytes_written = serialize_object(out, h);
				if (bytes_written == -1)
					return -1;
				total_bytes_written += bytes_written;

				return total_bytes_written;
			}

			std::ptrdiff_t AreaScaleData::deserialize(InStreamInterface& in)
			{
				std::ptrdiff_t total_bytes_read = 0;

				std::ptrdiff_t bytes_read = deserialize_object(in, x);
				if (bytes_read == -1)
					return -1;
				total_bytes_read += bytes_read;

				bytes_read = deserialize_object(in, y);
				if (bytes_read == -1)
					return -1;
				total_bytes_read += bytes_read;

				bytes_read = deserialize_object(in, w);
				if (bytes_read == -1) 
					return -1;
				total_bytes_read += bytes_read;

				bytes_read = deserialize_object(in, h);
				if (bytes_read == -1)
					return -1;
				total_bytes_read += bytes_read;

				return total_bytes_read;
			}

			std::ptrdiff_t ThemeResource::serialize(OutStreamInterface& out) const
			{
				std::ptrdiff_t total_bytes_written = 0;

				std::ptrdiff_t bytes_written = serialize_object(out, key);
				if (bytes_written == -1)
					return -1;
				total_bytes_written += bytes_written;

				bytes_written = serialize_object(out, animation);
				if (bytes_written == -1)
					return -1;
				total_bytes_written += bytes_written;

				bytes_written = serialize_object(out, area_scale);
				if (bytes_written == -1)
					return -1;
				total_bytes_written += bytes_written;

				return total_bytes_written;
			}

			std::ptrdiff_t ThemeResource::deserialize(InStreamInterface& in)
			{
				std::ptrdiff_t total_bytes_read = 0;

				std::ptrdiff_t bytes_read = deserialize_object(in, key);
				if (bytes_read == -1)
					return -1;
				total_bytes_read += bytes_read;

				bytes_read = deserialize_object(in, animation);
				if (bytes_read == -1)
					return -1;
				total_bytes_read += bytes_read;

				bytes_read = deserialize_object(in, area_scale);
				if (bytes_read == -1)
					return -1;
				total_bytes_read += bytes_read;

				return total_bytes_read;
			}

			std::ptrdiff_t ThemeConfig::serialize(OutStreamInterface& out) const
			{
				std::ptrdiff_t total_bytes_written = 0;

				std::ptrdiff_t bytes_written = serialize_object(out, name);
				if (bytes_written == -1)
					return -1;
				total_bytes_written += bytes_written;

				bytes_written = serialize_object(out, enable_built_in_font);
				if (bytes_written == -1)
					return -1;
				total_bytes_written += bytes_written;

				bytes_written = serialize_object(out, resources);
				if (bytes_written == -1)
					return -1;
				total_bytes_written += bytes_written;

				return total_bytes_written;
			}

			std::ptrdiff_t ThemeConfig::deserialize(InStreamInterface& in)
			{
				std::ptrdiff_t total_bytes_read = 0;

				std::ptrdiff_t bytes_read = deserialize_object(in, name);
				if (bytes_read == -1)
					return -1;
				total_bytes_read += bytes_read;

				bytes_read = deserialize_object(in, enable_built_in_font);
				if (bytes_read == -1)
					return -1;
				total_bytes_read += bytes_read;

				bytes_read = deserialize_object(in, resources);
				if (bytes_read == -1)
					return -1;
				total_bytes_read += bytes_read;

				return total_bytes_read;
			}

			std::ptrdiff_t ThemeGeneratorConfig::serialize(core::stream::OutStreamInterface& out) const
			{
				return serialize_object(out, resources);
			}

			std::ptrdiff_t ThemeGeneratorConfig::deserialize(core::stream::InStreamInterface& in)
			{
				return deserialize_object(in, resources);
			}
		}
	}
}