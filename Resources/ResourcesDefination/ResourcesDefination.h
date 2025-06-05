#pragma once
#include <string>
#include <cstdint>
#include <cstddef>
#include <Core/Serialization/Serialization.h>
#include <Core/Stream/Stream.h>

namespace cyanvne
{
	namespace resources
	{
		enum class ResourceType
		{
			CONFIG_DATA,

			FONT,

			IMAGE,
			AUDIO,

			HASH_MAP,
			TREE_MAP,
			VECTOR,
			LIST,
			HASH_SET,
			TREE_SET,

			TEXT,
			SCRIPT,

			UNKNOWN,
		};

		struct ResourceDefinition : public core::binaryserializer::BinarySerialiable
		{
			uint64_t id;
			std::string alias;

			uint64_t size;
			uint64_t offset;

			ResourceType type;

			ResourceDefinition() = default;

			ResourceDefinition(uint64_t id_val, std::string alias_val, uint64_t size_val, uint64_t offset_val, ResourceType type_val) :
				id(id_val), alias(std::move(alias_val)), size(size_val), offset(offset_val), type(type_val)
			{  }

			ResourceDefinition(const ResourceDefinition& other)
			{
                id = other.id;
                alias = other.alias;
				size = other.size;
                offset = other.offset;
                type = other.type;
			}
            ResourceDefinition(ResourceDefinition&& other) noexcept
			{
                id = other.id;
                alias = std::move(other.alias);
				size = other.size;
                offset = other.offset;
                type = other.type;
			}
			ResourceDefinition& operator=(const ResourceDefinition& other)
			{
                id = other.id;
                alias = other.alias;
                size = other.size;
                offset = other.offset;
                type = other.type;
                return *this;
			}
			ResourceDefinition& operator=(ResourceDefinition&& other) noexcept
			{
				id = other.id;
				alias = std::move(other.alias);
				size = other.size;
				offset = other.offset;
				type = other.type;
				return *this;
			}

			std::ptrdiff_t deserialize(cyanvne::core::stream::InStreamInterface& in) override
			{
				std::ptrdiff_t total_bytes_read = 0;

				std::ptrdiff_t bytes_read = core::binaryserializer::deserialize_object(in, id);
				if (bytes_read == -1)
				{
					return -1;
				}
				total_bytes_read += bytes_read;

				bytes_read = core::binaryserializer::deserialize_object(in, alias);
				if (bytes_read == -1)
				{
					return -1;
				}
				total_bytes_read += bytes_read;

				bytes_read = core::binaryserializer::deserialize_object(in, size);
				if (bytes_read == -1)
				{
					return -1;
				}
				total_bytes_read += bytes_read;

				bytes_read = core::binaryserializer::deserialize_object(in, offset);
				if (bytes_read == -1)
				{
					return -1;
				}
				total_bytes_read += bytes_read;

				bytes_read = core::binaryserializer::deserialize_object(in, type);
				if (bytes_read == -1)
				{
					return -1;
				}
				total_bytes_read += bytes_read;

				return total_bytes_read;
			}

			std::ptrdiff_t serialize(cyanvne::core::stream::OutStreamInterface& out) const override
			{
				std::ptrdiff_t total_bytes_written = 0;

				std::ptrdiff_t bytes_written = core::binaryserializer::serialize_object(out, id);
				if (bytes_written == -1)
				{
					return -1;
				}
				total_bytes_written += bytes_written;

				bytes_written = core::binaryserializer::serialize_object(out, alias);
				if (bytes_written == -1)
				{
					return -1;
				}
				total_bytes_written += bytes_written;

				bytes_written = core::binaryserializer::serialize_object(out, size);
				if (bytes_written == -1)
				{
					return -1;
				}
				total_bytes_written += bytes_written;

				bytes_written = core::binaryserializer::serialize_object(out, offset);
				if (bytes_written == -1)
				{
					return -1;
				}
				total_bytes_written += bytes_written;

				bytes_written = core::binaryserializer::serialize_object(out, type);
				if (bytes_written == -1)
				{
					return -1;
				}
				total_bytes_written += bytes_written;

				return total_bytes_written;
			}

			~ResourceDefinition() override = default;
		};

		struct ResourcesFileIdentificationHeader : public core::binaryserializer::BinarySerialiable
		{
			uint64_t version = 2;
			uint64_t magic = 0xACABFECFEDALLU;

			std::ptrdiff_t deserialize(core::stream::InStreamInterface& in) override
			{
				std::ptrdiff_t total_bytes_read = 0;

				std::ptrdiff_t bytes_read = core::binaryserializer::deserialize_object(in, version);
				if (bytes_read == -1)
				{
					return -1;
				}
				total_bytes_read += bytes_read;

				bytes_read = core::binaryserializer::deserialize_object(in, magic);
				if (bytes_read == -1)
				{
					return -1;
				}
				total_bytes_read += bytes_read;

				return total_bytes_read;
			}

			std::ptrdiff_t serialize(cyanvne::core::stream::OutStreamInterface& out) const override
			{
				std::ptrdiff_t total_bytes_written = 0;

				std::ptrdiff_t bytes_written = core::binaryserializer::serialize_object(out, version);
				if (bytes_written == -1)
				{
					return -1;
				}
				total_bytes_written += bytes_written;

				bytes_written = core::binaryserializer::serialize_object(out, magic);
				if (bytes_written == -1)
				{
					return -1;
				}
				total_bytes_written += bytes_written;

				return total_bytes_written;
			}
		};

		struct ResourcesFileHeader : public core::binaryserializer::BinarySerialiable
		{
			ResourcesFileIdentificationHeader identification_header_;
			uint64_t definition_offset_;

			std::ptrdiff_t deserialize(core::stream::InStreamInterface& in) override
			{
				std::ptrdiff_t total_bytes_read = 0;

				std::ptrdiff_t bytes_read = identification_header_.deserialize(in);
				if (bytes_read == -1)
				{
					return -1;
				}
				total_bytes_read += bytes_read;

				bytes_read = core::binaryserializer::deserialize_object(in, definition_offset_);
				if (bytes_read == -1)
				{
					return -1;
				}
				total_bytes_read += bytes_read;

				return total_bytes_read;
			}

			std::ptrdiff_t serialize(cyanvne::core::stream::OutStreamInterface& out) const override
			{
				std::ptrdiff_t total_bytes_written = 0;

				std::ptrdiff_t bytes_written = identification_header_.serialize(out);
				if (bytes_written == -1)
				{
					return -1;
				}
				total_bytes_written += bytes_written;

				bytes_written = core::binaryserializer::serialize_object(out, definition_offset_);
				if (bytes_written == -1)
				{
					return -1;
				}
				total_bytes_written += bytes_written;

				return total_bytes_written;
			}
		};
	}
}