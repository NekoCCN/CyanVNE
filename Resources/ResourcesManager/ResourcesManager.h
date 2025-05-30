#pragma once
#include <Core/Stream/Stream.h>
#include <Resources/ResourcesDefination/ResourcesDefination.h>
#include <Resources/ResourcesException/ResourcesException.h>
#include <Core/MemoryStreamImpl/MemoryStreamImpl.h>
#include <vector>
#include <string>
#include <map>
#include <cstdint>
#include <memory>
#include <stdexcept>

namespace cyanvne
{
    namespace resources
    {
        class ResourcesManager
        {
        private:
            std::shared_ptr<core::stream::InStreamInterface> in_stream_;

            ResourcesFileHeader file_header_;
            std::vector<ResourceDefinition> definitions_;

            std::map<uint64_t, uint64_t> id_to_definition_index_;
            std::map<std::string, uint64_t> alias_to_id_;

            bool initialized_ = false;

            void loadDefinitions();

        public:
            explicit ResourcesManager(std::shared_ptr<core::stream::InStreamInterface> stream);

            ~ResourcesManager() = default;

            ResourcesManager(const ResourcesManager&) = delete;
            ResourcesManager& operator=(const ResourcesManager&) = delete;
            ResourcesManager(ResourcesManager&&) = delete;
            ResourcesManager& operator=(ResourcesManager&&) = delete;

            bool isInitialized() const
            {
                return initialized_;
            }

            const ResourceDefinition* getDefinitionById(uint64_t id) const;

            const ResourceDefinition* getDefinitionByAlias(const std::string& alias) const;

            std::vector<uint8_t> getResourceDataById(uint64_t id) const;

            std::vector<uint8_t> getResourceDataByAlias(const std::string& alias) const;

            std::shared_ptr<core::stream::FixedSizeMemoryStreamImpl> getResourceAsStreamById(uint64_t id) const;

            std::shared_ptr<core::stream::FixedSizeMemoryStreamImpl> getResourceAsStreamByAlias(const std::string& alias) const;

            const std::vector<ResourceDefinition>& getAllDefinitions() const
            {
                if (!initialized_)
                {
                    throw exception::IllegalStateException("ResourcesManager not initialized, cannot get definitions.");
                }
                return definitions_;
            }
        };
    }
}