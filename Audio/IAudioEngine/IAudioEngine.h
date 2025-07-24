//
// Created by Ext Culith on 2025/7/20.
//

#ifndef IAUDIOENGINE_H
#define IAUDIOENGINE_H

#include <string>
#include <string_view>

namespace cyanvne::resources
{ class SoLoudWavResource; }

namespace cyanvne::audio
{
    using VoiceHandle = unsigned int;
    using BusHandle = unsigned int;

    class IAudioEngine
    {
    public:
        virtual ~IAudioEngine() = default;

        virtual void update() = 0;

        virtual BusHandle createBus(std::string_view name) = 0;
        virtual BusHandle getBus(std::string_view name) const = 0;
        virtual void setBusVolume(BusHandle bus, float volume) = 0;
        virtual void fadeBusVolume(BusHandle bus, float target_volume, float time_seconds) = 0;

        virtual VoiceHandle play(BusHandle bus, resources::SoLoudWavResource* sound, float volume = 1.0f, bool paused = false) = 0;
        virtual VoiceHandle playPositional(BusHandle bus, resources::SoLoudWavResource* sound, float x, float y, float z, float volume = 1.0f, bool paused = false) = 0;

        virtual void playDetached(resources::SoLoudWavResource* sound, float volume = 1.0f) = 0;

        virtual void stop(VoiceHandle handle) = 0;
        virtual void setVolume(VoiceHandle handle, float volume) = 0;
        virtual void setPaused(VoiceHandle handle, bool is_paused) = 0;
        virtual bool getPaused(VoiceHandle handle) const = 0;
        virtual void fadeVolume(VoiceHandle handle, float target_volume, float time_seconds) = 0;
        virtual void scheduleStop(VoiceHandle handle, float time_seconds) = 0;
        virtual float getPlaybackTime(VoiceHandle handle) const = 0;
        virtual void seek(VoiceHandle handle, float time_seconds) = 0;

        virtual void updateListenerPosition(float x, float y, float z = 0.0f) = 0;
        virtual void updateSoundPosition(VoiceHandle handle, float x, float y, float z = 0.0f) = 0;

        virtual void stopBus(BusHandle bus) = 0;

        virtual void pauseBus(BusHandle bus) = 0;

        virtual void resumeBus(BusHandle bus) = 0;
    };
}

#endif //IAUDIOENGINE_H
