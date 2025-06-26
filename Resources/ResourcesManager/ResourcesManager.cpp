#include "ResourcesManager.h"
#include <Core/Stream/Stream.h>
#include <Core/Serialization/Serialization.h>
#include "ResourcesException/ResourcesException.h"

void cyanvne::resources::ResourcesManager::loadDefinitions()
{
	std::lock_guard<std::mutex> lock(stream_mutex_);

	if (!in_stream_ || !in_stream_->is_open())
	{
		throw exception::resourcesexception::ResourceManagerIOException("Input stream is not valid or not open for loading definitions.");
	}
	if (in_stream_->seek(static_cast<int64_t>(file_header_.definition_offset_), core::stream::SeekMode::Begin) == -1)
	{
		throw exception::resourcesexception::ResourceManagerIOException("Failed to seek to definition offset in resource pack.");
	}
	if (core::binaryserializer::deserialize_object(*in_stream_, definitions_) < 0)
	{
		throw exception::resourcesexception::ResourceManagerIOException("Failed to deserialize resource definitions vector.");
	}
	if (core::binaryserializer::deserialize_object(*in_stream_, id_to_definition_index_) < 0)
	{
		throw exception::resourcesexception::ResourceManagerIOException("Failed to deserialize id_to_definition_index_ map.");
	}
	if (core::binaryserializer::deserialize_object(*in_stream_, alias_to_id_) < 0)
	{
		throw exception::resourcesexception::ResourceManagerIOException("Failed to deserialize alias_to_id_ map.");
	}
}

cyanvne::resources::ResourcesManager::ResourcesManager(std::shared_ptr<core::stream::InStreamInterface> stream)
	: in_stream_(std::move(stream)), initialized_(false)
{
	if (!in_stream_ || !in_stream_->is_open())
	{
		throw exception::resourcesexception::ResourceManagerIOException("Input stream is not valid or not open for ResourcesManager.");
	}
	{
		std::lock_guard<std::mutex> lock(stream_mutex_);
		if (core::binaryserializer::deserialize_object(*in_stream_, file_header_) < 0)
		{
			throw exception::resourcesexception::ResourceManagerIOException("Failed to read resource pack file header.");
		}
	}

	ResourcesFileIdentificationHeader expected_id_header;
	if (file_header_.identification_header_.magic != expected_id_header.magic)
	{
		throw exception::resourcesexception::ResourceManagerIOException("Resource pack magic number mismatch.");
	}
	if (file_header_.identification_header_.version != expected_id_header.version)
	{
		throw exception::resourcesexception::ResourceManagerIOException("Resource pack version mismatch. Expected: " + std::to_string(expected_id_header.version) + ", Got: " + std::to_string(file_header_.identification_header_.version) + ".");
	}
	loadDefinitions();
	initialized_ = true;
}

const cyanvne::resources::ResourceDefinition* cyanvne::resources::ResourcesManager::getDefinitionById(uint64_t id) const
{
	if (!initialized_)
	{
		throw exception::IllegalStateException("ResourcesManager is not initialized.");
	}
	auto it = id_to_definition_index_.find(id);
	if (it != id_to_definition_index_.end())
	{
		uint64_t definition_idx = it->second;
		if (definition_idx < definitions_.size())
		{
			return &definitions_[definition_idx];
		}
	}
	return nullptr;
}

const cyanvne::resources::ResourceDefinition* cyanvne::resources::ResourcesManager::getDefinitionByAlias(const std::string& alias) const
{
	if (!initialized_)
	{
		throw exception::IllegalStateException("ResourcesManager not initialized.");
	}
	if (alias.empty())
	{
		return nullptr;
	}
	auto it_alias_to_id = alias_to_id_.find(alias);
	if (it_alias_to_id == alias_to_id_.end())
	{
		return nullptr;
	}
	return getDefinitionById(it_alias_to_id->second);
}

std::vector<uint8_t> cyanvne::resources::ResourcesManager::getResourceDataById(uint64_t id) const
{
	const ResourceDefinition* def = getDefinitionById(id);
	if (!def)
	{
		throw exception::resourcesexception::ResourceManagerIOException("Resource not found for ID: " + std::to_string(id) + ".");
	}

	std::vector<uint8_t> data_buffer(def->size);
	if (def->size > 0)
	{
		std::lock_guard<std::mutex> lock(stream_mutex_);
		if (!in_stream_ || !in_stream_->is_open())
		{
			throw exception::resourcesexception::ResourceManagerIOException("Input stream is not available for reading resource data.");
		}
		if (in_stream_->seek(static_cast<int64_t>(def->offset), core::stream::SeekMode::Begin) == -1)
		{
			throw exception::resourcesexception::ResourceManagerIOException("Failed to seek to resource offset for ID: " + std::to_string(id) + ".");
		}
		size_t bytes_actually_read = in_stream_->read(data_buffer.data(), def->size);
		if (bytes_actually_read != def->size)
		{
			throw exception::resourcesexception::ResourceManagerIOException("Failed to read complete resource data for ID: " + std::to_string(id) + ". Expected " + std::to_string(def->size) + " bytes, Got " + std::to_string(bytes_actually_read) + " bytes.");
		}
	}
	return data_buffer;
}

std::vector<uint8_t> cyanvne::resources::ResourcesManager::getResourceDataByAlias(const std::string& alias) const
{
	const ResourceDefinition* def = getDefinitionByAlias(alias);
	if (!def)
	{
		throw exception::resourcesexception::ResourceManagerIOException("Resource not found for alias: " + alias + ".");
	}
	return getResourceDataById(def->id);
}

std::shared_ptr<cyanvne::core::stream::InStreamInterface> cyanvne::resources::ResourcesManager::openResourceStreamById(uint64_t id) const
{
	const ResourceDefinition* def = getDefinitionById(id);
	if (!def)
	{
		throw exception::resourcesexception::ResourceManagerIOException("Resource not found for ID: " + std::to_string(id) + ".");
	}
	return std::make_shared<cyanvne::core::stream::SubStream>(in_stream_, stream_mutex_, def->offset, def->size);
}

std::shared_ptr<cyanvne::core::stream::InStreamInterface> cyanvne::resources::ResourcesManager::openResourceStreamByAlias(const std::string& alias) const
{
	const ResourceDefinition* def = getDefinitionByAlias(alias);
	if (!def)
	{
		throw exception::resourcesexception::ResourceManagerIOException("Resource not found for alias: " + alias + ".");
	}
	return std::make_shared<cyanvne::core::stream::SubStream>(in_stream_, stream_mutex_, def->offset, def->size);
}

const std::vector<cyanvne::resources::ResourceDefinition>& cyanvne::resources::ResourcesManager::getAllDefinitions() const
{
	if (!initialized_)
	{
		throw exception::resourcesexception::ThemeResourceManagerIOException("ThemeResourcesManager not initialized, cannot get definitions.");
	}
	return definitions_;
}
