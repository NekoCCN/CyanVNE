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
#include "Runtime/Components/Components.h"

namespace cyanvne::audio
{
    /**
     * @brief Manages audio playback and resources using a specific audio engine.
     *
     * This is a template class constrained by the `AudioEngine` concept. The specific
     * engine (e.g., SoloudAudioEngine) must be provided at compile time.
     * @tparam T The audio engine type, which must satisfy the `audio::AudioEngine` concept.
     */
    template <AudioEngine T>
    class AudioManager
    {
    public:
        // Constructor now takes the concrete engine type by value and moves it.
        AudioManager(T engine,
                     std::shared_ptr<resources::UnifiedCacheManager> cache_manager,
                     platform::EventBus& event_bus);
        ~AudioManager();

        // Deleted copy constructor and assignment operator to prevent copying.
        AudioManager(const AudioManager&) = delete;
        AudioManager& operator=(const AudioManager&) = delete;

        std::shared_ptr<resources::UnifiedCacheManager> getCacheManager() const { return cache_manager_; }

        // Connects the manager to an entity-component-system registry.
        void connectToRegistry(entt::registry& registry);
        void disconnectFromRegistry();

        // Calls the update method of the underlying audio engine.
        // This should be called once per frame in the main game loop.
        void update();

    private:
        // Callback for when an AudioSourceComponent is removed from an entity.
        void onAudioSourceRemoved(entt::registry& registry, entt::entity entity);

        // Event handlers for audio control.
        void handlePlayAudioEvent(const events::PlayAudioEvent& event);
        void handleAudioBusControlEvent(const events::AudioBusControlEvent& event);
        void handleVoiceControlEvent(const events::VoiceControlEvent& event);

        // The audio engine instance, its type is determined at compile time.
        T engine_;
        std::shared_ptr<resources::UnifiedCacheManager> cache_manager_;
        platform::EventBus& event_bus_;

        entt::connection remove_connection_;
        std::vector<platform::Subscription> subscriptions_;

        // Maps a string tag to a currently playing voice for easy control.
        std::map<std::string, VoiceHandle> tagged_voices_;
    };

    // --- Implementation of the template class ---

    template <AudioEngine T>
    AudioManager<T>::AudioManager(T engine,
                                  std::shared_ptr<resources::UnifiedCacheManager> cache_manager,
                                  platform::EventBus& event_bus)
            : engine_(std::move(engine)),
              cache_manager_(std::move(cache_manager)),
              event_bus_(event_bus)
    {
        // Subscribe to all relevant audio events upon construction.
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
        // Directly call the engine's update method.
        engine_.update();
    }

    template <AudioEngine T>
    void AudioManager<T>::connectToRegistry(entt::registry& registry)
    {
        disconnectFromRegistry();
        // Connect the onAudioSourceRemoved method to the registry's destruction signal for AudioSourceComponent.
        remove_connection_ = registry.on_destroy<ecs::AudioSourceComponent>().connect<&AudioManager<T>::onAudioSourceRemoved>(this);
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
        // When an audio source component is removed, stop its associated sound.
        auto& source = registry.get<ecs::AudioSourceComponent>(entity);
        if (source.voice_handle != 0)
        {
            engine_.stop(source.voice_handle);
        }
    }

    template <AudioEngine T>
    void AudioManager<T>::handlePlayAudioEvent(const events::PlayAudioEvent& event)
    {
        try
        {
            auto sound_res = cache_manager_->get<resources::SoLoudWavResource>(event.resource_key);
            if (!sound_res) return;

            // Ensure the bus exists, creating it if necessary.
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

            // If a tag is provided, manage the tagged voice.
            if (event.tag.has_value() && handle != 0)
            {
                // If a sound with the same tag is already playing, stop it.
                if (tagged_voices_.contains(event.tag.value()))
                {
                    engine_.stop(tagged_voices_[event.tag.value()]);
                }
                tagged_voices_[event.tag.value()] = handle;
            }
        }
        catch (const std::exception& e)
        {
            core::GlobalLogger::getCoreLogger()->error("AudioManager: Failed to handle PlayAudioEvent for '{}': {}", event.resource_key, e.what());
        }
    }

    template <AudioEngine T>
    void AudioManager<T>::handleAudioBusControlEvent(const events::AudioBusControlEvent& event)
    {
        auto bus_handle = engine_.getBus(event.bus_name);
        if (bus_handle == 0 && event.bus_name != "Master")
        {
            core::GlobalLogger::getCoreLogger()->warn("AudioManager: Attempted to control non-existent bus '{}'", event.bus_name);
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
            core::GlobalLogger::getCoreLogger()->warn("AudioManager: Attempted to control a voice with non-existent tag '{}'", event.tag);
            return;
        }

        VoiceHandle handle = it->second;
        bool remove_tag_after = false;

        switch (event.action)
        {
            case events::VoiceControlEvent::Action::STOP:
                engine_.stop(handle);
                remove_tag_after = true;
                break;
            case events::VoiceControlEvent::Action::FADE_OUT_AND_STOP:
                engine_.fadeVolume(handle, 0.0f, event.value2);
                engine_.scheduleStop(handle, event.value2);
                remove_tag_after = true;
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

        if (remove_tag_after)
        {
            tagged_voices_.erase(it);
        }
    }
}

#endif //AUDIOMANAGER_H
