//
// Created by Ext Culith on 2025/7/20.
//

#include "SoloudAudioEngine.h"
#include <soloud_wav.h>
#include "Resources/ResourceTypes/ResourceTypes.h"

namespace cyanvne::audio
{
    SoloudAudioEngine::SoloudAudioEngine() : soloud_(std::make_unique<SoLoud::Soloud>())
    {
        soloud_->init();
    }

    SoloudAudioEngine::~SoloudAudioEngine()
    {
        if (soloud_)
        {
            soloud_->deinit();
        }
    }

    void SoloudAudioEngine::update()
    {
        soloud_->update3dAudio();
    }

    BusHandle SoloudAudioEngine::createBus(std::string_view name)
    {
        std::string name_str(name);
        if (bus_map_.contains(name_str))
        {
            return bus_map_.at(name_str);
        }
        BusHandle handle = soloud_->createVoiceGroup();
        bus_map_[name_str] = handle;
        return handle;
    }

    BusHandle SoloudAudioEngine::getBus(std::string_view name) const
    {
        auto it = bus_map_.find(std::string(name));
        return (it != bus_map_.end()) ? it->second : 0;
    }

    void SoloudAudioEngine::setBusVolume(BusHandle bus, float volume)
    {
        soloud_->setVolume(bus, volume);
    }

    void SoloudAudioEngine::fadeBusVolume(BusHandle bus, float target_volume, float time_seconds)
    {
        soloud_->fadeVolume(bus, target_volume, time_seconds);
    }

    VoiceHandle SoloudAudioEngine::play(BusHandle bus, resources::SoLoudWavResource* sound, float volume, bool paused)
    {
        if (!sound) return 0;
        VoiceHandle handle = soloud_->play(sound->sound, volume, 0, paused);
        if (bus != 0) // 0是主总线，默认就在其中，无需添加
        {
            soloud_->addVoiceToGroup(bus, handle);
        }
        return handle;
    }

    VoiceHandle SoloudAudioEngine::playPositional(BusHandle bus, resources::SoLoudWavResource* sound, float x, float y, float z, float volume, bool paused)
    {
        if (!sound)
            return 0;
        VoiceHandle handle = soloud_->play3d(sound->sound, x, y, z, 0, 0, 0, volume, paused);
        if (bus != 0)
        {
            soloud_->addVoiceToGroup(bus, handle);
        }
        return handle;
    }

    void SoloudAudioEngine::playDetached(resources::SoLoudWavResource* sound, float volume)
    {
        if (!sound)
            return;

        soloud_->play(sound->sound, volume);
    }

    void SoloudAudioEngine::stop(VoiceHandle handle)
    {
        if (handle != 0 && soloud_->isValidVoiceHandle(handle))
        {
            soloud_->stop(handle);
        }
    }

    void SoloudAudioEngine::setVolume(VoiceHandle handle, float volume)
    {
        if (handle != 0 && soloud_->isValidVoiceHandle(handle))
        {
            soloud_->setVolume(handle, volume);
        }
    }

    void SoloudAudioEngine::setPaused(VoiceHandle handle, bool is_paused)
    {
        if (handle != 0 && soloud_->isValidVoiceHandle(handle))
        {
            soloud_->setPause(handle, is_paused);
        }
    }

    bool SoloudAudioEngine::getPaused(VoiceHandle handle) const
    {
        if (handle != 0 && soloud_->isValidVoiceHandle(handle))
        {
            return soloud_->getPause(handle);
        }
        return false;
    }

    void SoloudAudioEngine::fadeVolume(VoiceHandle handle, float target_volume, float time_seconds)
    {
        if (handle != 0 && soloud_->isValidVoiceHandle(handle))
        {
            soloud_->fadeVolume(handle, target_volume, time_seconds);
        }
    }

    void SoloudAudioEngine::scheduleStop(VoiceHandle handle, float time_seconds)
    {
        if (handle != 0 && soloud_->isValidVoiceHandle(handle))
        {
            soloud_->scheduleStop(handle, time_seconds);
        }
    }

    float SoloudAudioEngine::getPlaybackTime(VoiceHandle handle) const
    {
        if (handle != 0 && soloud_->isValidVoiceHandle(handle))
        {
            return soloud_->getStreamTime(handle);
        }
        return 0.0f;
    }

    void SoloudAudioEngine::seek(VoiceHandle handle, float time_seconds)
    {
        if (handle != 0 && soloud_->isValidVoiceHandle(handle))
        {
            soloud_->seek(handle, time_seconds);
        }
    }

    void SoloudAudioEngine::updateListenerPosition(float x, float y, float z)
    {
        soloud_->set3dListenerPosition(x, y, z);
    }

    void SoloudAudioEngine::updateSoundPosition(VoiceHandle handle, float x, float y, float z)
    {
        if (handle != 0 && soloud_->isValidVoiceHandle(handle))
        {
            soloud_->set3dSourcePosition(handle, x, y, z);
        }
    }
}