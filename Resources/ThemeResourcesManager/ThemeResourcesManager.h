#pragma once
#include <variant>
#include <Resources/UnifiedCacheManager/UnifiedCacheManager.h>
#include <Resources/ICacheResourcesManager/ICacheResourcesManager.h>

namespace cyanvne
{
    namespace resources
    {
        class ThemeResourcesManager
        {
        private:
            std::shared_ptr<ResourcesManager> base_manager_;
            std::shared_ptr<UnifiedCacheManager> cache_manager_;
            parser::theme::ThemeConfig theme_config_;
            bool initialized_ = false;

        public:
            explicit ThemeResourcesManager(
                const std::shared_ptr<ResourcesManager>& base_manager,
                uint64_t max_volatile_size, SDL_Renderer* renderer): base_manager_(base_manager)
            {
	            if (!base_manager || !base_manager->isInitialized())
	            {
		            throw exception::resourcesexception::ResourceManagerIOException("Provided base ResourcesManager is null or not initialized.");
	            }

                cache_manager_ = std::make_shared<UnifiedCacheManager>(base_manager, renderer, max_volatile_size);
	            if (!cache_manager_)
	            {
		            throw exception::resourcesexception::ResourceManagerIOException("Provided UnifiedCacheManager is null.");
	            }

	            try
	            {
		            DataHandle config_handle = cache_manager_->get<RawDataResource>("theme_config");

		            auto mem_stream = std::make_shared<core::stream::FixedSizeMemoryStreamImpl>(
			            config_handle->data.data(),
			            config_handle->data.size()
		            );

		            if (theme_config_.deserialize(*mem_stream) < 0)
		            {
			            throw exception::resourcesexception::ResourceManagerIOException("Failed to deserialize ThemeConfig.");
		            }
	            }
	            catch (const std::exception& e)
	            {
		            throw exception::resourcesexception::ResourceManagerIOException("Failed to load or parse 'theme_config': " + std::string(e.what()));
	            }

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

            const parser::theme::ThemeConfig& getThemeConfig() const
            {
                if (!initialized_)
                {
                    throw exception::resourcesexception::ResourceManagerIOException("ThemeResourcesManager is not initialized.");
                }
                return theme_config_;
            }

            TextureHandle getTexture(const std::string& alias) const
            {
                if (!initialized_) throw exception::resourcesexception::ResourceManagerIOException("ThemeResourcesManager is not initialized.");
                return cache_manager_->get<TextureResource>(alias);
            }

            DataHandle getData(const std::string& alias) const
            {
                if (!initialized_) throw exception::resourcesexception::ResourceManagerIOException("ThemeResourcesManager is not initialized.");
                return cache_manager_->get<RawDataResource>(alias);
            }

            SoundHandle getSound(const std::string& alias) const
            {
                if (!initialized_) throw exception::resourcesexception::ResourceManagerIOException("ThemeResourcesManager is not initialized.");
                return cache_manager_->get<SoLoudWavResource>(alias);
            }

            std::variant<SoundHandle, StreamHandle> getSoundOrStream(const std::string& alias) const
            {
                if (!initialized_)
                {
                    throw exception::resourcesexception::ResourceManagerIOException("ThemeResourcesManager is not initialized.");
                }

                if (base_manager_->getDefinitionByAlias(alias)->size * 3 > cache_manager_->getMaxCacheBufferSize())
                {
                    core::GlobalLogger::getCoreLogger()->warn("Resource '{}' likely too large for cache, falling back to streaming.", alias);
                    return base_manager_->openResourceStreamByAlias(alias);
                }

                try
                {
                    return cache_manager_->get<SoLoudWavResource>(alias);
                }
                catch (const exception::MemoryAllocException& e)
                {
                    core::GlobalLogger::getCoreLogger()->warn("Resource '{}' likely too large for cache, falling back to streaming. Reason: {}", alias, e.what());
                    return base_manager_->openResourceStreamByAlias(alias);
                }
            }

            std::variant<DataHandle, StreamHandle> getDataOrStream(const std::string& alias) const
            {
                if (!initialized_)
                {
                    throw exception::resourcesexception::ResourceManagerIOException("ThemeResourcesManager is not initialized.");
                }

                if (base_manager_->getDefinitionByAlias(alias)->size * 3 > cache_manager_->getMaxCacheBufferSize())
                {
                    core::GlobalLogger::getCoreLogger()->warn("Resource '{}' likely too large for cache, falling back to streaming.", alias);
                    return base_manager_->openResourceStreamByAlias(alias);
                }

                try
                {
                    return cache_manager_->get<RawDataResource>(alias);
                }
                catch (const exception::MemoryAllocException& e)
                {
                    core::GlobalLogger::getCoreLogger()->warn("Resource '{}' likely too large for cache, falling back to streaming. Reason: {}", alias, e.what());
                    return base_manager_->openResourceStreamByAlias(alias);
                }
            }
        };
    }
}
