#ifndef AUDIOMANAGER_H
#define AUDIOMANAGER_H

#include "Platform/EventBus/EventBus.h"
#include "Resources/UnifiedCacheManager/UnifiedCacheManager.h"
#include <entt/entt.hpp>
#include <memory>
#include <vector>
#include <string>
#include <map>
#include <optional>
#include "Audio/IAudioEngine/IAudioEngine.h"
#include "Runtime/Components/Components.h"
#include "Audio/Events/AudioEvents.h"

namespace cyanvne::audio
{
    template <AudioEngine T>
    class AudioManager
    {
    public:
        AudioManager(T engine,
                     std::shared_ptr<resources::UnifiedCacheManager> cache_manager,
                     platform::EventBus& event_bus);
        ~AudioManager();

        AudioManager(const AudioManager&) = delete;
        AudioManager& operator=(const AudioManager&) = delete;

        std::shared_ptr<resources::UnifiedCacheManager> getCacheManager() const { return cache_manager_; }

        void connectToRegistry(entt::registry& registry);
        void disconnectFromRegistry();

        void update();

        T& getEngine() { return engine_; }

    private:
        void onAudioSourceRemoved(entt::registry& registry, entt::entity entity);

        void handlePlayAudioEvent(const events::PlayAudioEvent& event);
        void handleAudioBusControlEvent(const events::AudioBusControlEvent& event);
        void handleVoiceControlEvent(const events::VoiceControlEvent& event);

        T engine_;
        std::shared_ptr<resources::UnifiedCacheManager> cache_manager_;
        platform::EventBus& event_bus_;

        entt::connection remove_connection_;
        std::vector<platform::Subscription> subscriptions_;

        std::map<std::string, VoiceHandle> tagged_voices_;

        struct ActiveVoice {
            resources::ResourceHandle<resources::SoLoudWavResource> resource;
            VoiceHandle handle;
        };

        std::map<VoiceHandle, ActiveVoice> active_voices_;
    };

    template <AudioEngine T>
    AudioManager<T>::AudioManager(T engine,
                                  std::shared_ptr<resources::UnifiedCacheManager> cache_manager,
                                  platform::EventBus& event_bus)
            : engine_(std::move(engine)),
              cache_manager_(std::move(cache_manager)),
              event_bus_(event_bus)
    {
        subscriptions_.push_back(
                event_bus_.subscribe<events::PlayAudioEvent>([this](const auto& e){ this->handlePlayAudioEvent(e); return false; })
        );
        subscriptions_.push_back(
                event_bus_.subscribe<events::AudioBusControlEvent>([this](const auto& e){ this->handleAudioBusControlEvent(e); return false; })
        );
        subscriptions_.push_back(
                event_bus_.subscribe<events::VoiceControlEvent>([this](const auto& e){ this->handleVoiceControlEvent(e); return false; })
        );
    }

    template <AudioEngine T>
    AudioManager<T>::~AudioManager()
    {
        disconnectFromRegistry();
    }

    template <AudioEngine T>
    void AudioManager<T>::update()
    {
        engine_.update();

        auto it = active_voices_.begin();
        while (it != active_voices_.end())
        {
            if (!engine_.isValidVoiceHandle(it->first))
            {
                it = active_voices_.erase(it);
            }
            else
            {
                ++it;
            }
        }

        auto tag_it = tagged_voices_.begin();
        while (tag_it != tagged_voices_.end())
        {
            if (!engine_.isValidVoiceHandle(tag_it->second))
            {
                tag_it = tagged_voices_.erase(tag_it);
            }
            else
            {
                ++tag_it;
            }
        }
    }

    template <AudioEngine T>
    void AudioManager<T>::connectToRegistry(entt::registry& registry)
    {
        disconnectFromRegistry();
        remove_connection_ = registry.on_destroy<runtime::AudioSourceComponent>().connect<&AudioManager<T>::onAudioSourceRemoved>(this);
    }

    template <AudioEngine T>
    void AudioManager<T>::disconnectFromRegistry()
    {
        if (remove_connection_)
        {
            remove_connection_.release();
        }
    }

    template <AudioEngine T>
    void AudioManager<T>::onAudioSourceRemoved(entt::registry& registry, entt::entity entity)
    {
        auto& source = registry.get<runtime::AudioSourceComponent>(entity);
        if (source.voice_handle != 0)
        {
            engine_.stop(source.voice_handle);
            active_voices_.erase(source.voice_handle);
        }
    }

    template <AudioEngine T>
    void AudioManager<T>::handlePlayAudioEvent(const events::PlayAudioEvent& event)
    {
        try
        {
            auto sound_res = cache_manager_->get<resources::SoLoudWavResource>(event.resource_key);
            if (!sound_res)
            {
                core::GlobalLogger::getCoreLogger()->warn("AudioManager: Failed to load audio resource '{}'", event.resource_key);
                return;
            }

            auto bus_handle = engine_.getBus(event.bus_name);
            if (bus_handle == 0 && event.bus_name != "Main")
            {
                bus_handle = engine_.createBus(event.bus_name);
            }

            sound_res->sound.setLooping(event.loop);

            VoiceHandle handle = 0;
            if (event.is_positional)
            {
                handle = engine_.playPositional(bus_handle, sound_res.get(), event.position.x, event.position.y, 0.0f, event.volume);
            }
            else
            {
                handle = engine_.play(bus_handle, sound_res.get(), event.volume);
            }

            if (handle != 0)
            {
                active_voices_.emplace(handle, ActiveVoice{ std::move(sound_res), handle });

                if (event.tag.has_value())
                {
                    const std::string& tag = event.tag.value();
                    if (tagged_voices_.contains(tag))
                    {
                        VoiceHandle old_handle = tagged_voices_[tag];
                        engine_.stop(old_handle);
                        active_voices_.erase(old_handle);
                    }
                    tagged_voices_[tag] = handle;
                }
            }
        }
        catch (const std::exception& e)
        {
            core::GlobalLogger::getCoreLogger()->error("AudioManager: Exception processing PlayAudioEvent for '{}': {}", event.resource_key, e.what());
        }
    }

    template <AudioEngine T>
    void AudioManager<T>::handleAudioBusControlEvent(const events::AudioBusControlEvent& event)
    {
        auto bus_handle = engine_.getBus(event.bus_name);
        if (bus_handle == 0 && event.bus_name != "Master")
        {
            return;
        }

        switch (event.action)
        {
            case events::AudioBusControlEvent::Action::SET_VOLUME:
                engine_.setBusVolume(bus_handle, event.value1);
                break;
            case events::AudioBusControlEvent::Action::FADE_VOLUME:
                engine_.fadeBusVolume(bus_handle, event.value1, event.value2);
                break;
            case events::AudioBusControlEvent::Action::STOP_ALL:
                engine_.stopBus(bus_handle);
                break;
            case events::AudioBusControlEvent::Action::PAUSE_ALL:
                engine_.pauseBus(bus_handle);
                break;
            case events::AudioBusControlEvent::Action::RESUME_ALL:
                engine_.resumeBus(bus_handle);
                break;
        }
    }

    template <AudioEngine T>
    void AudioManager<T>::handleVoiceControlEvent(const events::VoiceControlEvent& event)
    {
        auto it = tagged_voices_.find(event.tag);
        if (it == tagged_voices_.end())
        {
            return;
        }

        VoiceHandle handle = it->second;

        if (!engine_.isValidVoiceHandle(handle))
        {
            tagged_voices_.erase(it);
            return;
        }

        switch (event.action)
        {
            case events::VoiceControlEvent::Action::STOP:
                engine_.stop(handle);
                active_voices_.erase(handle);
                tagged_voices_.erase(it);
                break;
            case events::VoiceControlEvent::Action::FADE_OUT_AND_STOP:
                engine_.fadeVolume(handle, 0.0f, event.value2);
                engine_.scheduleStop(handle, event.value2);
                break;
            case events::VoiceControlEvent::Action::SET_VOLUME:
                engine_.setVolume(handle, event.value1);
                break;
            case events::VoiceControlEvent::Action::FADE_VOLUME:
                engine_.fadeVolume(handle, event.value1, event.value2);
                break;
            case events::VoiceControlEvent::Action::SET_PAUSED:
                engine_.setPaused(handle, static_cast<bool>(event.value1));
                break;
            case events::VoiceControlEvent::Action::SEEK:
                engine_.seek(handle, event.value1);
                break;
        }
    }
}

#endif //AUDIOMANAGER_H