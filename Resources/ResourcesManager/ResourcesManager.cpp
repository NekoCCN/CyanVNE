#include "ResourcesManager.h"
#include "ResourcesException/ResourcesException.h"

namespace cyanvne
{
    namespace resources
    {
        ResourcesManager::ResourcesManager(const std::string& resource_file_path, std::shared_ptr<core::IPathToStream> path_to_stream)
                : resource_file_path_(resource_file_path), path_to_stream_(std::move(path_to_stream)), initialized_(false)
        {
            if (!path_to_stream_)
            {
                throw exception::resourcesexception::ResourceManagerIOException("IPathToStream provider is null.");
            }

            auto in_stream = path_to_stream_->getInStream(resource_file_path_);
            if (!in_stream || !in_stream->is_open())
            {
                throw exception::resourcesexception::ResourceManagerIOException("Input stream is not valid or not open for ResourcesManager initialization.");
            }

            if (core::binaryserializer::deserialize_object(*in_stream, file_header_) < 0)
            {
                throw exception::resourcesexception::ResourceManagerIOException("Failed to read resource pack file header.");
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

            if (in_stream->seek(static_cast<int64_t>(file_header_.definition_offset_), core::stream::SeekMode::Begin) == -1)
            {
                throw exception::resourcesexception::ResourceManagerIOException("Failed to seek to definition offset in resource pack.");
            }
            if (core::binaryserializer::deserialize_object(*in_stream, definitions_) < 0)
            {
                throw exception::resourcesexception::ResourceManagerIOException("Failed to deserialize resource definitions vector.");
            }
            if (core::binaryserializer::deserialize_object(*in_stream, id_to_definition_index_) < 0)
            {
                throw exception::resourcesexception::ResourceManagerIOException("Failed to deserialize id_to_definition_index_ map.");
            }
            if (core::binaryserializer::deserialize_object(*in_stream, alias_to_id_) < 0)
            {
                throw exception::resourcesexception::ResourceManagerIOException("Failed to deserialize alias_to_id_ map.");
            }

            initialized_ = true;
        }

        const ResourceDefinition* ResourcesManager::getDefinitionById(uint64_t id) const
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

        const ResourceDefinition* ResourcesManager::getDefinitionByAlias(const std::string& alias) const
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

        std::vector<uint8_t> ResourcesManager::getResourceDataById(uint64_t id) const
        {
            const ResourceDefinition* def = getDefinitionById(id);
            if (!def)
            {
                throw exception::resourcesexception::ResourceManagerIOException("Resource not found for ID: " + std::to_string(id) + ".");
            }

            std::vector<uint8_t> data_buffer(def->size);
            if (def->size > 0)
            {
                auto in_stream = path_to_stream_->getInStream(resource_file_path_);
                if (!in_stream || !in_stream->is_open())
                {
                    throw exception::resourcesexception::ResourceManagerIOException("Input stream is not available for reading resource data.");
                }
                if (in_stream->seek(static_cast<int64_t>(def->offset), core::stream::SeekMode::Begin) == -1)
                {
                    throw exception::resourcesexception::ResourceManagerIOException("Failed to seek to resource offset for ID: " + std::to_string(id) + ".");
                }
                size_t bytes_actually_read = in_stream->read(data_buffer.data(), def->size);
                if (bytes_actually_read != def->size)
                {
                    throw exception::resourcesexception::ResourceManagerIOException("Failed to read complete resource data for ID: " + std::to_string(id) + ". Expected " + std::to_string(def->size) + " bytes, Got " + std::to_string(bytes_actually_read) + " bytes.");
                }
            }
            return data_buffer;
        }

        std::vector<uint8_t> ResourcesManager::getResourceDataByAlias(const std::string& alias) const
        {
            const ResourceDefinition* def = getDefinitionByAlias(alias);
            if (!def)
            {
                throw exception::resourcesexception::ResourceManagerIOException("Resource not found for alias: " + alias + ".");
            }
            return getResourceDataById(def->id);
        }

        std::shared_ptr<core::stream::InStreamInterface> ResourcesManager::openResourceStreamById(uint64_t id) const
        {
            const ResourceDefinition* def = getDefinitionById(id);
            if (!def)
            {
                throw exception::resourcesexception::ResourceManagerIOException("Resource not found for ID: " + std::to_string(id) + ".");
            }

            auto full_stream = path_to_stream_->getInStream(resource_file_path_);
            if (!full_stream || !full_stream->is_open())
            {
                throw exception::resourcesexception::ResourceManagerIOException("Failed to open new stream for resource ID: " + std::to_string(id));
            }

            return std::make_shared<core::stream::SubStream>(full_stream, def->offset, def->size);
        }

        std::shared_ptr<core::stream::InStreamInterface> ResourcesManager::openResourceStreamByAlias(const std::string& alias) const
        {
            const ResourceDefinition* def = getDefinitionByAlias(alias);
            if (!def)
            {
                throw exception::resourcesexception::ResourceManagerIOException("Resource not found for alias: " + alias + ".");
            }
            return openResourceStreamById(def->id);
        }

        const std::vector<ResourceDefinition>& ResourcesManager::getAllDefinitions() const
        {
            if (!initialized_)
            {
                throw exception::IllegalStateException("ResourcesManager not initialized, cannot get definitions.");
            }
            return definitions_;
        }
    }
}