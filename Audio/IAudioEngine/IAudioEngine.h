//
// Created by Ext Culith on 2025/7/20.
//

#ifndef IAUDIOENGINE_H
#define IAUDIOENGINE_H

#include <string>
#include <string_view>

namespace cyanvne::resources
{
    class SoLoudWavResource;
}

namespace cyanvne::audio
{
    // Type aliases remain the same.
    using VoiceHandle = unsigned int;
    using BusHandle = unsigned int;

    template<typename T>
    concept AudioEngine = requires(T engine, const T const_engine, BusHandle bus, VoiceHandle handle, resources::SoLoudWavResource* sound, float f, bool b, float x, float y, float z, std::string_view sv)
    {
        // General engine update
        { engine.update() } -> std::same_as<void>;

        // Bus Management
        { engine.createBus(sv) } -> std::same_as<BusHandle>;
        { const_engine.getBus(sv) } -> std::same_as<BusHandle>;
        { engine.setBusVolume(bus, f) } -> std::same_as<void>;
        { engine.fadeBusVolume(bus, f, f) } -> std::same_as<void>;
        { engine.stopBus(bus) } -> std::same_as<void>;
        { engine.pauseBus(bus) } -> std::same_as<void>;
        { engine.resumeBus(bus) } -> std::same_as<void>;

        // Sound Playback
        { engine.play(bus, sound, f, b) } -> std::same_as<VoiceHandle>;
        { engine.playPositional(bus, sound, x, y, z, f, b) } -> std::same_as<VoiceHandle>;
        { engine.playDetached(sound, f) } -> std::same_as<void>;

        // Voice Control
        { engine.stop(handle) } -> std::same_as<void>;
        { engine.setVolume(handle, f) } -> std::same_as<void>;
        { engine.setPaused(handle, b) } -> std::same_as<void>;
        { const_engine.getPaused(handle) } -> std::same_as<bool>;
        { engine.fadeVolume(handle, f, f) } -> std::same_as<void>;
        { engine.scheduleStop(handle, f) } -> std::same_as<void>;
        { const_engine.getPlaybackTime(handle) } -> std::same_as<float>;
        { engine.seek(handle, f) } -> std::same_as<void>;

        // 3D Audio Control
        { engine.updateListenerPosition(x, y, z) } -> std::same_as<void>;
        { engine.updateSoundPosition(handle, x, y, z) } -> std::same_as<void>;
    };
}

#endif //IAUDIOENGINE_H
