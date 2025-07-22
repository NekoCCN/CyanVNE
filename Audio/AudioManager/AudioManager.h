//
// Created by Ext Culith on 2025/7/21.
//

#ifndef AUDIOMANAGER_H
#define AUDIOMANAGER_H

#include "Platform/EventBus/EventBus.h"
#include "Resources/UnifiedCacheManager/UnifiedCacheManager.h"
#include "Runtime/Events/AudioEvents.h"
#include <entt/entt.hpp>
#include <memory>
#include <vector>
#include <string>
#include <map>
#include "Audio/IAudioEngine/IAudioEngine.h"

namespace cyanvne::audio
{
    class AudioManager
    {
    public:
        AudioManager(std::unique_ptr<IAudioEngine> engine,
                     std::shared_ptr<resources::UnifiedCacheManager> cache_manager,
                     platform::EventBus& event_bus);
        ~AudioManager();

        AudioManager(const AudioManager&) = delete;
        AudioManager& operator=(const AudioManager&) = delete;

        IAudioEngine* getEngine() const { return engine_.get(); }
        std::shared_ptr<resources::UnifiedCacheManager> getCacheManager() const { return cache_manager_; }

        void connectToRegistry(entt::registry& registry);
        void disconnectFromRegistry();

    private:
        void onAudioSourceRemoved(entt::registry& registry, entt::entity entity);

        void handlePlayAudioEvent(const events::PlayAudioEvent& event);
        void handleAudioBusControlEvent(const events::AudioBusControlEvent& event);
        void handleVoiceControlEvent(const events::VoiceControlEvent& event);

        std::unique_ptr<IAudioEngine> engine_;
        std::shared_ptr<resources::UnifiedCacheManager> cache_manager_;
        platform::EventBus& event_bus_;

        entt::connection remove_connection_;
        std::vector<platform::Subscription> subscriptions_;

        std::map<std::string, VoiceHandle> tagged_voices_;
    };
}

#endif //AUDIOMANAGER_H
