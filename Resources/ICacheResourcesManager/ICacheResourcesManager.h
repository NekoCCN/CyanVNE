#pragma once
#include <variant>
#include <Resources/UnifiedCacheManager/UnifiedCacheManager.h>

namespace cyanvne
{
    namespace resources
    {
        using TextureHandle = ResourceHandle<TextureResource>;
        using SoundHandle = ResourceHandle<SoLoudWavResource>;
        using DataHandle = ResourceHandle<RawDataResource>;
        using StreamHandle = std::shared_ptr<core::stream::InStreamInterface>;

        class ICacheResourcesManager
        {
        protected:
            ICacheResourcesManager() = default;
        public:
			ICacheResourcesManager(const ICacheResourcesManager&) = delete;
            ICacheResourcesManager(ICacheResourcesManager&&) = delete;
            ICacheResourcesManager& operator=(const ICacheResourcesManager&) = delete;
            ICacheResourcesManager& operator=(ICacheResourcesManager&&) = delete;

            virtual TextureHandle getTexture(const std::string& alias) const = 0;
            virtual DataHandle getData(const std::string& alias) const = 0;
            virtual SoundHandle getSound(const std::string& alias) const = 0;
            virtual std::variant<SoundHandle, StreamHandle> getSoundOrStream(const std::string& alias) const = 0;
            virtual std::variant<DataHandle, StreamHandle> getDataOrStream(const std::string& alias) const = 0;

            virtual ~ICacheResourcesManager() = default;
        };
    }
}
