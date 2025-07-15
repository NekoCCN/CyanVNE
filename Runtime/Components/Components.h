#pragma once
#include <string>
#include <vector>
#include <SDL3/SDL.h>
#include <entt/entt.hpp>
#include <Runtime/ICommand/ICommand.h>

namespace cyanvne::ecs
{
    /**
     * @brief 标识符组件。
     * 用于标记实体的唯一标识符，通常用于在系统中快速查找和引用实体。
     */
    struct IdentifierComponent
    {
        std::string id;
    };

    /**
     * @brief 布局组件。
     * 用于定义实体在窗口中的布局位置和大小比例。
     * area_ratio 是一个比例矩形，anchor 定义了该矩形的锚点位置。
     */
    struct LayoutComponent
    {
        SDL_FRect area_ratio{ 0.5f, 0.5f, 0.1f, 0.1f };
        SDL_FPoint anchor{ 0.5f, 0.5f };
    };


    /**
     * @brief 父实体组件。
     * 标记一个实体拥有一个父实体。
     */
    struct ParentComponent {
        entt::entity parent = entt::null;
    };

    /**
     * @brief 子实体列表组件。
     * 存储一个实体的所有子实体，方便快速遍历和关系管理。
     */
    struct ChildrenComponent {
        std::vector<entt::entity> children;
    };

    /**
     * @brief 局部变换组件。
     * 存储实体相对于其父实体的“局部”位置、缩放和旋转。
     * 如果没有父实体，则相对于世界原点。
     */
    struct TransformComponent {
        SDL_FPoint position{ 0.0f, 0.0f };
        SDL_FPoint scale{ 1.0f, 1.0f };
        double rotation = 0.0;
    };

    struct FinalTransformComponent
    {
        SDL_FRect destination_rect;
    };

    struct SpriteComponent
    {
        std::string resource_key;
        SDL_FRect source_rect{ 0, 0, 0, 0 };

        SDL_Color color_mod{ 255, 255, 255, 255 }; // RGBA color modulation
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
        std::vector<std::shared_ptr<commands::ICommand>> on_left_click;
        std::vector<std::shared_ptr<commands::ICommand>> on_right_click;
    };

    struct KeyFocusComponent
    {
        std::map<SDL_Keycode, std::vector<std::shared_ptr<commands::ICommand>>> key_actions;
    };

    struct ScrollableComponent
    {
        std::vector<std::shared_ptr<commands::ICommand>> on_scroll_up;
        std::vector<std::shared_ptr<commands::ICommand>> on_scroll_down;
    };
}