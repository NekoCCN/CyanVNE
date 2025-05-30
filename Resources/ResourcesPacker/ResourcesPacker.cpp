#include "ResourcesPacker.h"

uint64_t cyanvne::resources::ResourcesPacker::addResourceByStream(
	std::shared_ptr<core::stream::InStreamInterface> resource_stream, const ResourceType& type,
	const std::string& optional_alias)
{
	if (finalized_)
	{
		throw exception::resourcesexception::ResourcePackerBeenFinalizedException(
			"Cannot add resources after pack has been finalized.");
	}
	if (!resource_stream || !resource_stream->is_open())
	{
		throw exception::resourcesexception::ResourcePackerIOException(
			"Input resource stream is not valid or not open.");
	}
	if (!out_stream_ || !out_stream_->is_open())
	{
		throw exception::resourcesexception::ResourcePackerIOException(
			"Packer output stream is not valid or not open.");
	}

	uint64_t current_id = next_available_id_++;

	if (!optional_alias.empty())
	{
		if (alias_to_id_.count(optional_alias))
		{
			throw exception::resourcesexception::ResourcePackerIOException(
				"Alias '" + optional_alias + "' already exists.");
		}
		alias_to_id_[optional_alias] = current_id;
	}

	int64_t resource_offset = out_stream_->tell();
	if (resource_offset == -1)
	{
		throw exception::resourcesexception::ResourcePackerIOException(
			"Failed to get current stream position for resource offset.");
	}

	if (resource_stream->seek(0, core::stream::SeekMode::Begin) == -1)
	{
		throw exception::resourcesexception::ResourcePackerIOException(
			"Failed to seek to beginning of resource stream.");
	}

	uint64_t bytes_copied = core::stream::utils::copy_stream_chunked(*resource_stream, *out_stream_);

	ResourceDefinition definition(current_id, optional_alias, bytes_copied, static_cast<uint64_t>(resource_offset), type);
	definitions_.push_back(definition);
	id_to_definition_index_[current_id] = definitions_.size() - 1;

	return current_id;
}

uint64_t cyanvne::resources::ResourcesPacker::addResourceByString(const std::string& content, const ResourceType& type,
	const std::string& optional_alias)
{
	if (finalized_)
	{
		throw exception::resourcesexception::ResourcePackerBeenFinalizedException(
			"Cannot add resources after pack has been finalized.");
	}
	if (!out_stream_ || !out_stream_->is_open())
	{
		throw exception::resourcesexception::ResourcePackerIOException(
			"Packer output stream is not valid or not open.");
	}

	uint64_t current_id = next_available_id_++;

	if (!optional_alias.empty())
	{
		if (alias_to_id_.count(optional_alias))
		{
			throw exception::resourcesexception::ResourcePackerIOException(
				"Alias '" + optional_alias + "' already exists.");
		}
		alias_to_id_[optional_alias] = current_id;
	}

	int64_t resource_offset = out_stream_->tell();
	if (resource_offset == -1)
	{
		throw exception::resourcesexception::ResourcePackerIOException(
			"Failed to get current stream position for resource offset.");
	}

	size_t bytes_to_write = content.length();
	size_t bytes_written = 0;
	if (bytes_to_write > 0)
	{
		bytes_written = out_stream_->write(content.data(), bytes_to_write);
		if (bytes_written != bytes_to_write)
		{
			throw exception::resourcesexception::ResourcePackerIOException(
				"Failed to write all string data to output stream for alias: " + optional_alias);
		}
	}

	ResourceDefinition definition(current_id, optional_alias, bytes_written, static_cast<uint64_t>(resource_offset), type);
	definitions_.push_back(definition);
	id_to_definition_index_[current_id] = definitions_.size() - 1;

	return current_id;
}

uint64_t cyanvne::resources::ResourcesPacker::addResourceByData(const std::vector<uint8_t>& data,
	const ResourceType& type, const std::string& optional_alias)
{
	if (finalized_)
	{
		throw exception::resourcesexception::ResourcePackerBeenFinalizedException(
			"Cannot add resources after pack has been finalized.");
	}
	if (!out_stream_ || !out_stream_->is_open())
	{
		throw exception::resourcesexception::ResourcePackerIOException(
			"Packer output stream is not valid or not open.");
	}

	uint64_t current_id = next_available_id_++;

	if (!optional_alias.empty())
	{
		if (alias_to_id_.count(optional_alias))
		{
			throw exception::resourcesexception::ResourcePackerIOException(
				"Alias '" + optional_alias + "' already exists.");
		}
		alias_to_id_[optional_alias] = current_id;
	}

	int64_t resource_offset = out_stream_->tell();
	if (resource_offset == -1)
	{
		throw exception::resourcesexception::ResourcePackerIOException(
			"Failed to get current stream position for resource offset.");
	}

	size_t bytes_to_write = data.size();
	size_t bytes_written = 0;
	if (bytes_to_write > 0)
	{
		bytes_written = out_stream_->write(data.data(), bytes_to_write);
		if (bytes_written != bytes_to_write)
		{
			throw exception::resourcesexception::ResourcePackerIOException(
				"Failed to write all vector data to output stream for alias: " + optional_alias);
		}
	}

	ResourceDefinition definition(current_id, optional_alias, bytes_written, static_cast<uint64_t>(resource_offset), type);
	definitions_.push_back(definition);
	id_to_definition_index_[current_id] = definitions_.size() - 1;

	return current_id;
}

void cyanvne::resources::ResourcesPacker::finalizePack()
{
	if (finalized_)
	{
		return;
	}
	if (!out_stream_ || !out_stream_->is_open())
	{
		throw exception::resourcesexception::ResourcePackerIOException(
			"Output stream not available or not open for finalizing pack.");
	}

	int64_t def_offset_pos = out_stream_->tell();
	if (def_offset_pos == -1)
	{
		throw exception::resourcesexception::ResourcePackerIOException(
			"Failed to get stream position for definition offset.");
	}
	header_.definition_offset_ = static_cast<uint64_t>(def_offset_pos);

	size_t num_definitions = definitions_.size();
	if (core::binaryserializer::serialize_object(*out_stream_, num_definitions) < 0)
	{
		throw exception::resourcesexception::ResourcePackerIOException(
			"Failed to serialize number of definitions.");
	}

	for (const auto& def : definitions_)
	{
		if (core::binaryserializer::serialize_object(*out_stream_, def) < 0)
		{
			throw exception::resourcesexception::ResourcePackerIOException(
				"Failed to serialize resource definition for ID: " + std::to_string(def.id));
		}
	}

	if (out_stream_->seek(0, core::stream::SeekMode::Begin) == -1)
	{
		throw exception::resourcesexception::ResourcePackerIOException(
			"Failed to seek to beginning of stream to write final header.");
	}

	if (core::binaryserializer::serialize_object(*out_stream_, header_) < 0)
	{
		throw exception::resourcesexception::ResourcePackerIOException(
			"Failed to write final updated header to output stream.");
	}

	out_stream_->flush();
	finalized_ = true;
}
