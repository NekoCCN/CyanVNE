#pragma once
#include <string>

namespace cyanvne::runtime
{
    namespace events
    {
        struct ToggleGuiWindowEvent
        {
            std::string window_id;
        };

        struct StoryFileChosenEvent
        {
            std::string file_path;
        };
    }
}