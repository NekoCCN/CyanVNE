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

namespace cyanvne::resources
{
    class SoLoudWavResource;
}

namespace cyanvne::audio
{
    class SoloudAudioEngine
    {
    public:
        SoloudAudioEngine();
        ~SoloudAudioEngine();

        // Non-copyable, but movable to be used in std::variant.
        SoloudAudioEngine(const SoloudAudioEngine&) = delete;
        SoloudAudioEngine& operator=(const SoloudAudioEngine&) = delete;
        SoloudAudioEngine(SoloudAudioEngine&&) = default;
        SoloudAudioEngine& operator=(SoloudAudioEngine&&) = default;

        void update();

        BusHandle createBus(std::string_view name);
        BusHandle getBus(std::string_view name) const;
        void setBusVolume(BusHandle bus, float volume);
        void fadeBusVolume(BusHandle bus, float target_volume, float time_seconds);

        VoiceHandle play(BusHandle bus, resources::SoLoudWavResource* sound, float volume = 1.0f, bool paused = false);
        VoiceHandle playPositional(BusHandle bus, resources::SoLoudWavResource* sound, float x, float y, float z, float volume = 1.0f, bool paused = false);
        void playDetached(resources::SoLoudWavResource* sound, float volume = 1.0f);

        void stop(VoiceHandle handle);
        void setVolume(VoiceHandle handle, float volume);
        void setPaused(VoiceHandle handle, bool is_paused);
        bool getPaused(VoiceHandle handle) const;
        void fadeVolume(VoiceHandle handle, float target_volume, float time_seconds);
        void scheduleStop(VoiceHandle handle, float time_seconds);
        float getPlaybackTime(VoiceHandle handle) const;
        void seek(VoiceHandle handle, float time_seconds);

        void updateListenerPosition(float x, float y, float z = 0.0f);
        void updateSoundPosition(VoiceHandle handle, float x, float y, float z = 0.0f);

        void stopBus(BusHandle bus);
        void pauseBus(BusHandle bus);
        void resumeBus(BusHandle bus);

    private:
        std::unique_ptr<SoLoud::Soloud> soloud_;

        std::map<std::string, BusHandle, std::less<>> bus_map_;
    };
}

#endif //SOLOUDAUDIOENGINE_H
