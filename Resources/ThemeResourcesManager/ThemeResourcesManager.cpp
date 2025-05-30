#include "ThemeResourcesManager.h"

void cyanvne::resources::ThemeResourcesManager::cacheAllResources(const ResourcesManager& base_manager)
{
    if (!base_manager.isInitialized())
    {
        throw exception::resourcesexception::ThemeResourceManagerIOException("Base ResourcesManager is not initialized for caching.");
    }

    definitions_copy_ = base_manager.getAllDefinitions();

    for (size_t i = 0; i < definitions_copy_.size(); ++i)
    {
        const auto& def = definitions_copy_[i];
        id_to_definition_idx_copy_[def.id] = i;
        if (!def.alias.empty())
        {
            alias_to_id_copy_[def.alias] = def.id;
        }
    }

    bool theme_config_found_and_deserialized = false;

    for (const auto& def : definitions_copy_)
    {
        std::vector<uint8_t> data = base_manager.getResourceDataById(def.id);

        if (def.alias == "theme_config" && def.type == ResourceType::CONFIG_DATA)
        {
            resource_data_cache_[def.id] = data;

            auto mem_stream = std::make_shared<core::stream::FixedSizeMemoryStreamImpl>(
                resource_data_cache_[def.id].data(), resource_data_cache_[def.id].size());

            if (cached_theme_config_.deserialize(*mem_stream) < 0)
            {
                throw exception::resourcesexception::ThemeResourceManagerIOException("Failed to deserialize ThemeConfig for alias 'theme_config'.");
            }
            theme_config_found_and_deserialized = true;
        }
        else
        {
            resource_data_cache_[def.id] = std::move(data);
        }
    }

    if (!theme_config_found_and_deserialized)
    {
        throw exception::resourcesexception::ThemeResourceManagerIOException("Required 'theme_config' resource not found or not of type CONFIG_DATA in the pack.");
    }
}

const cyanvne::resources::ResourceDefinition* cyanvne::resources::ThemeResourcesManager::getDefinitionById(uint64_t id) const
{
    if (!initialized_)
    {
        throw exception::resourcesexception::ThemeResourceManagerIOException("ThemeResourcesManager not initialized, cannot access definitions.");
    }

    auto it = id_to_definition_idx_copy_.find(id);
    if (it != id_to_definition_idx_copy_.end())
    {
        if (it->second < definitions_copy_.size())
        {
            return &definitions_copy_[it->second];
        }
    }
    return nullptr;
}

const cyanvne::resources::ResourceDefinition* cyanvne::resources::ThemeResourcesManager::getDefinitionByAlias(const std::string& alias) const
{
    if (!initialized_)
    {
        throw exception::resourcesexception::ThemeResourceManagerIOException("ThemeResourcesManager not initialized, cannot access definitions.");
    }

    if (alias.empty())
    {
        return nullptr;
    }

    auto it_alias = alias_to_id_copy_.find(alias);
    if (it_alias != alias_to_id_copy_.end())
    {
        return getDefinitionById(it_alias->second);
    }
    return nullptr;
}

const std::vector<uint8_t>& cyanvne::resources::ThemeResourcesManager::getResourceDataById(uint64_t id) const
{
    if (!initialized_)
    {
        throw exception::resourcesexception::ThemeResourceManagerIOException("ThemeResourcesManager not initialized.");
    }

    auto it = resource_data_cache_.find(id);
    if (it == resource_data_cache_.end())
    {
        throw exception::resourcesexception::ThemeResourceManagerIOException("Resource data not found in cache for ID: " + std::to_string(id) + ".");
    }
    return it->second;
}

const std::vector<uint8_t>& cyanvne::resources::ThemeResourcesManager::getResourceDataByAlias(const std::string& alias) const
{
    if (!initialized_)
    {
        throw exception::resourcesexception::ThemeResourceManagerIOException("ThemeResourcesManager not initialized.");
    }

    const ResourceDefinition* def = getDefinitionByAlias(alias);
    if (!def)
    {
        throw exception::resourcesexception::ThemeResourceManagerIOException("Resource not found for alias: " + alias + ".");
    }
    return getResourceDataById(def->id);
}

std::shared_ptr<cyanvne::core::stream::FixedSizeMemoryStreamImpl> cyanvne::resources::ThemeResourcesManager::getResourceAsStreamById(uint64_t id) const
{
    const std::vector<uint8_t>& data = getResourceDataById(id);
    return std::make_shared<core::stream::FixedSizeMemoryStreamImpl>(data.data(), data.size());
}

std::shared_ptr<cyanvne::core::stream::FixedSizeMemoryStreamImpl> cyanvne::resources::ThemeResourcesManager::getResourceAsStreamByAlias(const std::string& alias) const
{
    const std::vector<uint8_t>& data = getResourceDataByAlias(alias);
    return std::make_shared<core::stream::FixedSizeMemoryStreamImpl>(data.data(), data.size());
}