#pragma once
#include <Parser/ThemeConfig/ThemeConfig.h>
#include <Resources/ResourcesException/ResourcesException.h>
#include <variant>
#include <Resources/CachingResourcesManager/CachingResourcesManager.h>

namespace cyanvne
{
    namespace resources
    {
        class ThemeResourcesManager
        {
        public:
            using ResourceResult = std::variant<AssetHandle, std::shared_ptr<core::stream::InStreamInterface>>;

        private:
            std::shared_ptr<ResourcesManager> base_manager_;
            std::unique_ptr<CachingResourcesManager> caching_manager_;
            parser::theme::ThemeConfig theme_config_;
            bool initialized_ = false;

        public:
            explicit ThemeResourcesManager(
                const std::shared_ptr<ResourcesManager>& base_manager,
                size_t max_volatile_size,
                size_t max_persistent_size,
                size_t max_single_persistent_size)
            {
                if (!base_manager)
                {
                    throw exception::resourcesexception::ThemeResourceManagerIOException("Provided base ResourcesManager is null.");
                }

                base_manager_ = base_manager;

                caching_manager_ = std::make_unique<CachingResourcesManager>(
                    base_manager_,
                    max_volatile_size,
                    max_persistent_size,
                    max_single_persistent_size
                );

                try
                {
                    auto config_handle = caching_manager_->getResource("theme_config");
                    auto mem_stream = std::make_shared<core::stream::FixedSizeMemoryStreamImpl>(
                        config_handle.getData().data(),
                        config_handle.getData().size()
                    );

                    if (theme_config_.deserialize(*mem_stream) < 0)
                    {
                        throw exception::resourcesexception::ThemeResourceManagerIOException("Failed to deserialize ThemeConfig.");
                    }
                }
                catch (const std::exception& e)
                {
                    throw exception::resourcesexception::ThemeResourceManagerIOException("Failed to load or parse 'theme_config': " + std::string(e.what()));
                }

                core::GlobalLogger::getCoreLogger()->info("Theme Caching Config: Max Volatile : {}, Max Persistent : {}, Max Single Persistent : {}, Max Total : {}",
                    max_volatile_size, max_persistent_size,
                    max_single_persistent_size, max_persistent_size + max_volatile_size);

                initialized_ = true;
            }

            ThemeResourcesManager(const ThemeResourcesManager&) = delete;
            ThemeResourcesManager& operator=(const ThemeResourcesManager&) = delete;
            ThemeResourcesManager(ThemeResourcesManager&&) = delete;
            ThemeResourcesManager& operator=(ThemeResourcesManager&&) = delete;

            bool isInitialized() const
            {
                return initialized_;
            }

            const parser::theme::ThemeConfig& getThemeConfig() const
            {
                return theme_config_;
            }

            ResourceResult getResource(const std::string& alias, bool as_persistent = false) const
            {
                if (!initialized_)
                {
                    throw exception::resourcesexception::ThemeResourceManagerIOException("ThemeResourcesManager is not initialized.");
                }

                try
                {
                    return caching_manager_->getResource(alias, as_persistent);
                }
                catch (const exception::MemoryAllocException&)
                {
                    return base_manager_->openResourceStreamByAlias(alias);
                }
            }

            std::vector<uint8_t> getResourceData(const std::string& alias) const
            {
                return base_manager_->getResourceDataByAlias(alias);
            }

            ~ThemeResourcesManager() = default;
        };
    }
}
