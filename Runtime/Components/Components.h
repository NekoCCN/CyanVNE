#pragma once

#include <string>
#include <vector>
#include <deque>
#include <memory>
#include <map>
#include <SDL3/SDL.h>
#include <entt/entt.hpp>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/matrix_decompose.hpp>

namespace cyanvne::ecs
{
    namespace commands
    {
        class ICommand;
    }

    struct LocalTransformComponent
    {
        glm::vec2 position { 0.0f, 0.0f };
        glm::vec2 scale { 1.0f, 1.0f };
        float rotation_degrees = 0.0f;
        glm::vec2 anchor { 0.5f, 0.5f };
    };

    struct WorldTransformMatrixComponent
    {
        glm::mat3 matrix = glm::mat3(1.0f);
    };

    struct FinalTransformComponent
    {
        SDL_FRect destination_rect {  };
        double rotation = 0.0;
        SDL_FPoint center {  };
    };

    struct LayoutComponent
    {
        SDL_FRect area_ratio { 0.5f, 0.5f, 0.1f, 0.1f };
        SDL_FPoint anchor { 0.5f, 0.5f };
    };

    struct IdentifierComponent
    {
        std::string id;
    };

    struct ParentComponent
    {
        entt::entity parent = entt::null;
    };

    struct ChildrenComponent
    {
        std::vector<entt::entity> children;
    };

    struct VisibleComponent
    {  };

    struct SpriteComponent
    {
        std::string resource_key;
        SDL_FRect source_rect{ 0, 0, 0, 0 };
        SDL_Color color_mod{ 255, 255, 255, 255 };
        int layer = 0;
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

    struct CommandPacket
    {
        std::unique_ptr<commands::ICommand> command;
        entt::entity source_entity = entt::null;
    };

    using CommandQueue = std::deque<CommandPacket>;

    struct ClickableComponent
    {
        std::vector<std::shared_ptr<commands::ICommand>> on_left_click;
        std::vector<std::shared_ptr<commands::ICommand>> on_right_click;
    };

    struct HasKeyFocus { };

    struct KeyFocusComponent
    {
        std::map<SDL_Keycode, std::vector<std::shared_ptr<commands::ICommand>>> key_actions;
    };

    struct ScrollableComponent
    {
        std::vector<std::shared_ptr<commands::ICommand>> on_scroll_up;
        std::vector<std::shared_ptr<commands::ICommand>> on_scroll_down;
    };

    struct Tween
    {
        enum class Property
        {
            TRANSFORM_POSITION_X,
            TRANSFORM_POSITION_Y,
            SPRITE_ALPHA
        };
        enum class EaseType
        {
            LINEAR,
            EASE_IN_QUAD,
            EASE_OUT_QUAD,
            EASE_IN_OUT_QUAD,
            EASE_OUT_SINE
        };
        Property target_property;
        EaseType easing_type = EaseType::LINEAR;
        float start_value;
        float end_value;
        float duration;
        float elapsed_time = 0.0f;
        bool is_finished = false;
    };

    struct TweenListComponent
    {
        std::vector<Tween> tweens;
    };
}