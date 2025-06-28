#pragma once
#include <string>
#include <vector>
#include <SDL3/SDL.h>
#include <entt/entt.hpp>
#include <Runtime/ICommand/ICommand.h>

namespace cyanvne::ecs
{
    struct IdentifierComponent
    {
        std::string id;
    };

    struct LayoutComponent
    {
        SDL_FRect area_ratio{ 0.5f, 0.5f, 0.1f, 0.1f };
        SDL_FPoint anchor{ 0.5f, 0.5f };
    };

    struct RenderTransformComponent
    {
        SDL_FRect destination_rect;
    };

    struct SpriteComponent
    {
        std::string resource_key;
        SDL_FRect source_rect{ 0, 0, 0, 0 };
    };

    struct SpriteAnimationComponent
    {
        std::vector<SDL_FRect> frames;
        float frame_duration = 0.1f;
        float current_time = 0.0f;
        int current_frame = 0;
        bool is_playing = true;
        bool loop = true;
    };

    struct VisibleComponent
    {  };

    struct CommandPacket
    {
        std::unique_ptr<commands::ICommand> command;
        entt::entity source_entity = entt::null;
    };

    struct HasKeyFocus
    {  };

    using CommandQueue = std::deque<CommandPacket>;

    struct ClickableComponent
    {
        std::vector<std::unique_ptr<commands::ICommand>> on_left_click;
        std::vector<std::unique_ptr<commands::ICommand>> on_right_click;
    };

    struct KeyFocusComponent
    {
        std::map<SDL_Keycode, std::vector<std::unique_ptr<commands::ICommand>>> key_actions;
    };

    struct ScrollableComponent
    {
        std::vector<std::unique_ptr<commands::ICommand>> on_scroll_up;
        std::vector<std::unique_ptr<commands::ICommand>> on_scroll_down;
    };
}