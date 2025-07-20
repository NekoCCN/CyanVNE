//
// Created by Ext Culith on 2025/7/20.
//

#ifndef SOLOUDAUDIOENGINE_H
#define SOLOUDAUDIOENGINE_H

#include <soloud.h>
#include <memory>
#include <string>
#include <map>
#include <string_view>
#include "Audio/IAudioEngine/IAudioEngine.h"

namespace cyanvne::audio
{
    class SoloudAudioEngine : public IAudioEngine
    {
    public:
        SoloudAudioEngine();
        ~SoloudAudioEngine() override;

        SoloudAudioEngine(const SoloudAudioEngine&) = delete;
        SoloudAudioEngine& operator=(const SoloudAudioEngine&) = delete;

        void update() override;

        BusHandle createBus(std::string_view name) override;
        BusHandle getBus(std::string_view name) const override;
        void setBusVolume(BusHandle bus, float volume) override;
        void fadeBusVolume(BusHandle bus, float target_volume, float time_seconds) override;

        VoiceHandle play(BusHandle bus, resources::SoLoudWavResource* sound, float volume = 1.0f, bool paused = false) override;
        VoiceHandle playPositional(BusHandle bus, resources::SoLoudWavResource* sound, float x, float y, float z, float volume = 1.0f, bool paused = false) override;
        void playDetached(resources::SoLoudWavResource* sound, float volume = 1.0f) override;

        void stop(VoiceHandle handle) override;
        void setVolume(VoiceHandle handle, float volume) override;
        void setPaused(VoiceHandle handle, bool is_paused) override;
        bool getPaused(VoiceHandle handle) const override;
        void fadeVolume(VoiceHandle handle, float target_volume, float time_seconds) override;
        void scheduleStop(VoiceHandle handle, float time_seconds) override;
        float getPlaybackTime(VoiceHandle handle) const override;
        void seek(VoiceHandle handle, float time_seconds) override;

        void updateListenerPosition(float x, float y, float z = 0.0f) override;
        void updateSoundPosition(VoiceHandle handle, float x, float y, float z = 0.0f) override;

    private:
        std::unique_ptr<SoLoud::Soloud> soloud_;

        std::map<std::string, BusHandle> bus_map_;
    };
}



#endif //SOLOUDAUDIOENGINE_H
