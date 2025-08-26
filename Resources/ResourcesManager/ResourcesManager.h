#pragma once

#include <Core/Stream/Stream.h>
#include <Resources/ResourcesDefination/ResourcesDefination.h>
#include <Core/PathToStream/PathToStream.h>
#include <vector>
#include <string>
#include <map>
#include <cstdint>
#include <memory>

namespace cyanvne
{
    namespace resources
    {
        class ResourcesManager
        {
        private:
            std::shared_ptr<core::IPathToStream> path_to_stream_;
            std::string resource_file_path_;

            ResourcesFileHeader file_header_;
            std::vector<ResourceDefinition> definitions_;

            std::map<uint64_t, uint64_t> id_to_definition_index_;
            std::map<std::string, uint64_t> alias_to_id_;

            bool initialized_ = false;

            void loadDefinitions();
        public:
            explicit ResourcesManager(const std::string& resource_file_path, std::shared_ptr<core::IPathToStream> path_to_stream);
            ~ResourcesManager() = default;

            ResourcesManager(const ResourcesManager&) = delete;
            ResourcesManager& operator=(const ResourcesManager&) = delete;
            ResourcesManager(ResourcesManager&&) = delete;
            ResourcesManager& operator=(ResourcesManager&&) = delete;

            bool isInitialized() const { return initialized_; }

            const ResourceDefinition* getDefinitionById(uint64_t id) const;
            const ResourceDefinition* getDefinitionByAlias(const std::string& alias) const;

            std::vector<uint8_t> getResourceDataById(uint64_t id) const;
            std::vector<uint8_t> getResourceDataByAlias(const std::string& alias) const;

            std::shared_ptr<core::stream::InStreamInterface> openResourceStreamById(uint64_t id) const;
            std::shared_ptr<core::stream::InStreamInterface> openResourceStreamByAlias(const std::string& alias) const;

            const std::vector<ResourceDefinition>& getAllDefinitions() const;
        };
    }
}