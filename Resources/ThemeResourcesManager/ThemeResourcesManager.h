#pragma once
#include <Resources/ResourcesManager/ResourcesManager.h>
#include <Parser/ThemeConfig/ThemeConfig.h>
#include <Core/Stream/Stream.h>
#include <Core/MemoryStreamImpl/MemoryStreamImpl.h>
#include <string>
#include <vector>
#include <map>
#include <memory>
namespace cyanvne
{
    namespace resources
    {
        class ThemeResourcesManager
        {
        private:
            parser::ThemeConfig cached_theme_config_;
            std::map<uint64_t, std::vector<uint8_t>> resource_data_cache_;
            std::vector<ResourceDefinition> definitions_copy_;
            std::map<uint64_t, uint64_t> id_to_definition_idx_copy_;
            std::map<std::string, uint64_t> alias_to_id_copy_;
            bool initialized_ = false;
            void cacheAllResources(const ResourcesManager& base_manager);
        public:
            explicit ThemeResourcesManager(const ResourcesManager& manager_to_cache_from)
                : initialized_(false)
            {
                if (!manager_to_cache_from.isInitialized())
                {
                    throw exception::resourcesexception::ThemeResourceManagerIOException("Provided ResourcesManager is not initialized.");
                }
                cacheAllResources(manager_to_cache_from);
                initialized_ = true;
            }
            ~ThemeResourcesManager() = default;
            ThemeResourcesManager(const ThemeResourcesManager&) = delete;
            ThemeResourcesManager& operator=(const ThemeResourcesManager&) = delete;
            ThemeResourcesManager(ThemeResourcesManager&&) = delete;
            ThemeResourcesManager& operator=(ThemeResourcesManager&&) = delete;

            bool isInitialized() const
            {
                return initialized_;
            }
            const parser::ThemeConfig& getThemeConfig() const
            {
                if (!initialized_)
                {
                    throw exception::resourcesexception::ThemeResourceManagerIOException("ThemeResourcesManager not initialized, cannot access ThemeConfig.");
                }
                return cached_theme_config_;
            }
            const ResourceDefinition* getDefinitionById(uint64_t id) const;

            const ResourceDefinition* getDefinitionByAlias(const std::string& alias) const;

            const std::vector<uint8_t>& getResourceDataById(uint64_t id) const;

            const std::vector<uint8_t>& getResourceDataByAlias(const std::string& alias) const;

            std::shared_ptr<core::stream::FixedSizeMemoryStreamImpl> getResourceAsStreamById(uint64_t id) const;

            std::shared_ptr<core::stream::FixedSizeMemoryStreamImpl> getResourceAsStreamByAlias(const std::string& alias) const;
            
            const std::vector<ResourceDefinition>& getAllDefinitions() const
            {
                if (!initialized_)
                {
                    throw exception::resourcesexception::ThemeResourceManagerIOException("ThemeResourcesManager not initialized, cannot get definitions.");
                }
                return definitions_copy_;
            }
        };
    }
}