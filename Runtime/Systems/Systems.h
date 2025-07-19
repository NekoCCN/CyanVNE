#pragma once
#include "entt/entt.hpp"
#include <Resources/ICacheResourcesManager/ICacheResourcesManager.h>
#include <Platform/WindowContext/WindowContext.h>
#include <Runtime/GameStateManager/GameStateManager.h>
#include "Runtime/Components/Components.h"

namespace cyanvne::ecs::systems
{
    namespace Easing
    {
        inline float Linear(float t)
        {
            return t;
        }
        inline float EaseInQuad(float t)
        {
            return t * t;
        }
        inline float EaseOutQuad(float t)
        {
            return t * (2.0f - t);
        }
        inline float EaseInOutQuad(float t)
        {
            return t < 0.5f ? 2.0f * t * t : -1.0f + (4.0f - 2.0f * t) * t;
        }
        inline float EaseOutSine(float t)
        {
            return sin(t * (float)M_PI / 2.0f);
        }
    }

    void LayoutSystem(entt::registry& registry, const cyanvne::platform::WindowContext& window,
                      const cyanvne::resources::ICacheResourcesManager& cache_manager);

    void SetParent(entt::registry& registry, entt::entity child, entt::entity parent);

    void HierarchySystem(entt::registry& registry, const cyanvne::resources::ICacheResourcesManager& cache_manager);

    void UpdateWorldTransformRecursive(entt::registry& registry, entt::entity entity,
        const ecs::TransformComponent& parent_world_transform);

    void TransformSystem(entt::registry& registry, const platform::WindowContext& window);

    void AnimationSystem(entt::registry& registry, float delta_time);

    void RenderSystem(entt::registry& registry, const platform::WindowContext& window, const resources::ICacheResourcesManager& cache_manager);

    void CommandSystem(entt::registry& registry, runtime::GameStateManager& gsm, platform::EventBus& bus);

    class InteractionSystem
    {
    private:
        std::shared_ptr<entt::registry> registry_;
        std::shared_ptr<cyanvne::ecs::CommandQueue> command_queue_;

        void pushCommands(const std::vector<std::shared_ptr<commands::ICommand>>& commands, entt::entity source) const
        {
            for (const auto& cmd_ptr : commands)
            {
                if (cmd_ptr)
                {
                    command_queue_->push_back({ .command= cmd_ptr->clone(), .source_entity= source});
                }
            }
        }

    public:
        InteractionSystem(const std::shared_ptr<entt::registry>& registry, platform::EventBus& bus,
            const std::shared_ptr<cyanvne::ecs::CommandQueue>& command_queue)
            : registry_(registry), command_queue_(command_queue)
        {
            bus.subscribeSDL([this](const SDL_Event& event)
                {
                    return processInput(event);
                });
        }

        bool processInput(const SDL_Event& event) const
        {
            if (event.type == SDL_EVENT_MOUSE_BUTTON_UP)
            {
                float mouse_x = (float)event.button.x;
                float mouse_y = (float)event.button.y;
                auto view = registry_->view<const ClickableComponent, const FinalTransformComponent, const VisibleComponent>();
                bool processed = false;

                view.each([this, &event, mouse_x, mouse_y, &processed](entt::entity entity, const ClickableComponent& clickable,
                    const FinalTransformComponent& transform)
                    {
                        SDL_FPoint point = { mouse_x, mouse_y };
                        if (SDL_PointInRectFloat(&point, &transform.destination_rect))
                        {
                            if (event.button.button == SDL_BUTTON_LEFT)
                            {
                                pushCommands(clickable.on_left_click, entity);
                            }
                            else if (event.button.button == SDL_BUTTON_RIGHT)
                            {
                                pushCommands(clickable.on_right_click, entity);
                            }
                            processed = true;
                        }
                    });

                if (processed)
                {
                    return true;
                }
            }
            else if (event.type == SDL_EVENT_KEY_DOWN)
            {
                auto view = registry_->view<const KeyFocusComponent, const HasKeyFocus>();
                bool processed = false;

                view.each([this, &event, &processed](entt::entity entity, const KeyFocusComponent& key_focus)
                    {
                        if (auto it = key_focus.key_actions.find(event.key.key); it != key_focus.key_actions.end())
                        {
                            pushCommands(it->second, entity);
                            processed = true;
                        }
                    });

                if (processed)
                {
                    return true;
                }
            }
            else if (event.type == SDL_EVENT_MOUSE_WHEEL)
            {
                float mouse_x, mouse_y;
                SDL_GetMouseState(&mouse_x, &mouse_y);
                auto view = registry_->view<const ScrollableComponent, const FinalTransformComponent, const VisibleComponent>();
                bool processed = false;

                view.each([this, &event, mouse_x, mouse_y, &processed](entt::entity entity, const ScrollableComponent& scrollable,
                    const FinalTransformComponent& transform)
                    {
                        SDL_FPoint point = { mouse_x, mouse_y };
                        if (SDL_PointInRectFloat(&point, &transform.destination_rect))
                        {
                            if (event.wheel.y > 0)
                            {
                                pushCommands(scrollable.on_scroll_up, entity);
                            }
                            else if (event.wheel.y < 0)
                            {
                                pushCommands(scrollable.on_scroll_down, entity);
                            }
                            processed = true;
                        }
                    });

                if (processed)
                {
                    return true;
                }
            }

            return false;
        }
    };
}
