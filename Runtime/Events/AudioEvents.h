 //
// Created by Ext Culith on 2025/7/21.
//

#ifndef AUDIOEVENTS_H
#define AUDIOEVENTS_H

#include <string>
#include <glm/glm.hpp>
#include <optional>

namespace cyanvne::events
{
    struct PlayAudioEvent
    {
        std::string resource_key;

        std::string bus_name = "SFX";

        float volume = 1.0f;

        bool loop = false;

        std::optional<std::string> tag = std::nullopt;

        bool is_positional = false;

        glm::vec2 position{ 0.0f, 0.0f };
    };

    struct AudioBusControlEvent
    {
        enum class Action {
            SET_VOLUME,
            FADE_VOLUME,
            STOP_ALL,
            PAUSE_ALL,
            RESUME_ALL
        };

        std::string bus_name;

        Action action;

        float value1 = 0.0f;

        float value2 = 0.0f;
    };

    /**
     * @brief 统一的、通过标签控制单个声音实例的事件。
     */
    struct VoiceControlEvent
    {
        enum class Action
      {
            STOP,
            FADE_OUT_AND_STOP,
            SET_VOLUME,
            FADE_VOLUME,
            SET_PAUSED,
            SEEK
        };

        std::string tag;

        Action action;

        float value1 = 0.0f;

        float value2 = 0.0f;
    };
}

#endif //AUDIOEVENTS_H
