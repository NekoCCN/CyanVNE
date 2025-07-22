//
// Created by Ext Culith on 2025/7/21.
//

#include "AudioManager.h"
#include "Runtime/Components/Components.h"
#include "Core/Logger/Logger.h"

namespace cyanvne::audio
{
    AudioManager::AudioManager(std::unique_ptr<IAudioEngine> engine,
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

    AudioManager::~AudioManager()
    {
        disconnectFromRegistry();
    }

    void AudioManager::connectToRegistry(entt::registry& registry)
    {
        disconnectFromRegistry();
        remove_connection_ = registry.on_destroy<ecs::AudioSourceComponent>().connect<&AudioManager::onAudioSourceRemoved>(this);
    }

    void AudioManager::disconnectFromRegistry()
    {
        if (remove_connection_)
        {
            remove_connection_.release();
        }
    }

    void AudioManager::onAudioSourceRemoved(entt::registry& registry, entt::entity entity)
    {
        auto& source = registry.get<ecs::AudioSourceComponent>(entity);
        if (source.voice_handle != 0)
        {
            engine_->stop(source.voice_handle);
        }
    }

    void AudioManager::handlePlayAudioEvent(const events::PlayAudioEvent& event)
    {
        try
        {
            auto sound_res = cache_manager_->get<resources::SoLoudWavResource>(event.resource_key);
            if (!sound_res) return;

            // 确保总线存在
            auto bus_handle = engine_->getBus(event.bus_name);
            if (bus_handle == 0 && event.bus_name != "Main")
            {
                bus_handle = engine_->createBus(event.bus_name);
            }

            sound_res->sound.setLooping(event.loop);

            VoiceHandle handle = 0;
            if (event.is_positional)
            {
                handle = engine_->playPositional(bus_handle, sound_res.get(), event.position.x, event.position.y, 0.0f, event.volume);
            }
            else
            {
                handle = engine_->play(bus_handle, sound_res.get(), event.volume);
            }

            if (event.tag.has_value() && handle != 0)
            {
                if (tagged_voices_.contains(event.tag.value()))
                {
                    engine_->stop(tagged_voices_[event.tag.value()]);
                }
                tagged_voices_[event.tag.value()] = handle;
            }
        }
        catch (const std::exception& e)
        {
            core::GlobalLogger::getCoreLogger()->error("AudioManager: Failed to handle PlayAudioEvent for '{}': {}", event.resource_key, e.what());
        }
    }

    void AudioManager::handleAudioBusControlEvent(const events::AudioBusControlEvent& event)
    {
        auto bus_handle = engine_->getBus(event.bus_name);
        if (bus_handle == 0 && event.bus_name != "Master")
        {
             core::GlobalLogger::getCoreLogger()->warn("AudioManager: Attempted to control non-existent bus '{}'", event.bus_name);
             return;
        }

        switch (event.action)
        {
        case events::AudioBusControlEvent::Action::SET_VOLUME:
            engine_->setBusVolume(bus_handle, event.value1);
            break;
        case events::AudioBusControlEvent::Action::FADE_VOLUME:
            engine_->fadeBusVolume(bus_handle, event.value1, event.value2);
            break;
        case events::AudioBusControlEvent::Action::STOP_ALL:
            engine_->stopBus(bus_handle);
            break;
        case events::AudioBusControlEvent::Action::PAUSE_ALL:
            engine_->pauseBus(bus_handle);
            break;
        case events::AudioBusControlEvent::Action::RESUME_ALL:
            engine_->resumeBus(bus_handle);
            break;
        }
    }

    void AudioManager::handleVoiceControlEvent(const events::VoiceControlEvent& event)
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
            engine_->stop(handle);
            remove_tag_after = true;
            break;
        case events::VoiceControlEvent::Action::FADE_OUT_AND_STOP:
            engine_->fadeVolume(handle, 0.0f, event.value2);
            engine_->scheduleStop(handle, event.value2);
            remove_tag_after = true;
            break;
        case events::VoiceControlEvent::Action::SET_VOLUME:
            engine_->setVolume(handle, event.value1);
            break;
        case events::VoiceControlEvent::Action::FADE_VOLUME:
            engine_->fadeVolume(handle, event.value1, event.value2);
            break;
        case events::VoiceControlEvent::Action::SET_PAUSED:
            engine_->setPaused(handle, static_cast<bool>(event.value1));
            break;
        case events::VoiceControlEvent::Action::SEEK:
            engine_->seek(handle, event.value1);
            break;
        }

        if (remove_tag_after)
        {
            tagged_voices_.erase(it);
        }
    }
}
