#include "Systems.h"
#include "Runtime/Components/Components.h"
#include "Runtime/Scene/Scene.h"
#include "Runtime/ICommand/ICommand.h"
#include "Platform/WindowContext/WindowContext.h"
#include "Resources/ICacheResourcesManager/ICacheResourcesManager.h"
#include <algorithm>
#include <glm/gtx/matrix_transform_2d.hpp>

namespace cyanvne::ecs::systems
{
    namespace
    {
        void UpdateWorldTransformRecursive(entt::registry& registry, entt::entity entity, const glm::mat3& parent_world_matrix)
        {
            const auto& local_transform = registry.get<const ecs::LocalTransformComponent>(entity);

            glm::mat3 local_matrix = glm::translate(glm::mat3(1.0f), local_transform.position);
            local_matrix = glm::rotate(local_matrix, glm::radians(local_transform.rotation_degrees));
            local_matrix = glm::scale(local_matrix, local_transform.scale);

            glm::mat3 world_matrix = parent_world_matrix * local_matrix;

            registry.emplace_or_replace<ecs::WorldTransformMatrixComponent>(entity, world_matrix);

            if (auto* children_comp = registry.try_get<ecs::ChildrenComponent>(entity))
            {
                for (auto child : children_comp->children)
                {
                    if (registry.valid(child))
                    {
                        UpdateWorldTransformRecursive(registry, child, world_matrix);
                    }
                }
            }
        }
    }

    void HierarchySystem(entt::registry& registry)
    {
        auto root_view = registry.view<ecs::LocalTransformComponent>(entt::exclude<ecs::ParentComponent>);
        const glm::mat3 identity_matrix = glm::mat3(1.0f);

        for (auto entity : root_view)
        {
            UpdateWorldTransformRecursive(registry, entity, identity_matrix);
        }
    }

    void FinalRectSystem(entt::registry& registry, const platform::WindowContext& window, const resources::ICacheResourcesManager& cache_manager)
    {
        const SDL_Rect window_rect = window.getWindowRect();
        const auto window_w = static_cast<float>(window_rect.w);
        const auto window_h = static_cast<float>(window_rect.h);

        auto ui_view = registry.view<const ecs::LayoutComponent>();
        for (auto entity : ui_view)
        {
            const auto& layout = ui_view.get<const ecs::LayoutComponent>(entity);
            SDL_FRect final_rect;
            float target_w_px = layout.area_ratio.w * window_w;
            float target_h_px = layout.area_ratio.h * window_h;

            if ((target_w_px <= 0.0f && target_h_px > 0.0f) || (target_w_px > 0.0f && target_h_px <= 0.0f))
            {
                if (auto* sprite = registry.try_get<ecs::SpriteComponent>(entity))
                {
                    auto texture_handle = cache_manager.getTexture(sprite->resource_key);
                    if (texture_handle.get() && texture_handle.get()->texture)
                    {
                        float tex_w = 0.0f, tex_h = 0.0f;
                        if (SDL_GetTextureSize(texture_handle.get()->texture, &tex_w, &tex_h) == 0 && tex_w > 0.0f && tex_h > 0.0f)
                        {
                            if (target_w_px > 0.0f)
                            {
                                target_h_px = target_w_px * (tex_h / tex_w);
                            }
                            else
                            {
                                target_w_px = target_h_px * (tex_w / tex_h);
                            }
                        }
                    }
                }
            }
            final_rect.w = target_w_px;
            final_rect.h = target_h_px;
            final_rect.x = (layout.area_ratio.x * window_w) - (final_rect.w * layout.anchor.x);
            final_rect.y = (layout.area_ratio.y * window_h) - (final_rect.h * layout.anchor.y);

            SDL_FPoint center = { final_rect.w * layout.anchor.x, final_rect.h * layout.anchor.y };
            registry.emplace_or_replace<ecs::FinalTransformComponent>(entity, final_rect, 0.0, center);
        }

        auto world_view = registry.view<const ecs::WorldTransformMatrixComponent, const ecs::LocalTransformComponent>(entt::exclude<ecs::LayoutComponent>);
        for (auto entity : world_view)
        {
            const auto& world_matrix = world_view.get<const ecs::WorldTransformMatrixComponent>(entity).matrix;
            const auto& local_transform = world_view.get<const ecs::LocalTransformComponent>(entity);

            const auto world_pos = glm::vec2(world_matrix[2]);

            glm::vec2 world_scale;
            world_scale.x = glm::length(glm::vec2(world_matrix[0]));
            world_scale.y = glm::length(glm::vec2(world_matrix[1]));

            double world_rotation_degrees = glm::degrees(atan2(world_matrix[0][1], world_matrix[0][0]));

            float tex_w = 64.0f, tex_h = 64.0f;
            if (auto* sprite = registry.try_get<const ecs::SpriteComponent>(entity))
            {
                auto texture_handle = cache_manager.getTexture(sprite->resource_key);
                if (texture_handle.get() && texture_handle.get()->texture)
                {
                    float w = 0.0f, h = 0.0f;
                    if (SDL_GetTextureSize(texture_handle.get()->texture, &w, &h) == 0)
                    {
                        tex_w = w;
                        tex_h = h;
                    }
                }
            }

            SDL_FRect final_rect;
            final_rect.w = tex_w * world_scale.x;
            final_rect.h = tex_h * world_scale.y;
            final_rect.x = world_pos.x - (final_rect.w * local_transform.anchor.x);
            final_rect.y = world_pos.y - (final_rect.h * local_transform.anchor.y);

            SDL_FPoint center = { final_rect.w * local_transform.anchor.x, final_rect.h * local_transform.anchor.y };

            registry.emplace_or_replace<ecs::FinalTransformComponent>(entity, final_rect, world_rotation_degrees, center);
        }
    }

    void RenderSystem(entt::registry& registry, const platform::WindowContext& window, const resources::ICacheResourcesManager& cache_manager)
    {
        SDL_Renderer* renderer = window.getRendererHinding();
        registry.sort<ecs::SpriteComponent>([](const auto& lhs, const auto& rhs) {
            return lhs.layer < rhs.layer;
        });

        auto view = registry.view<const ecs::FinalTransformComponent, const ecs::SpriteComponent, const ecs::VisibleComponent>();
        for (auto entity : view)
        {
            const auto& transform = view.get<const ecs::FinalTransformComponent>(entity);
            const auto& sprite = view.get<const ecs::SpriteComponent>(entity);

            auto texture_handle = cache_manager.getTexture(sprite.resource_key);
            if (!texture_handle.get() || !texture_handle.get()->texture)
            {
                continue;
            }
            SDL_Texture* texture = texture_handle.get()->texture;

            SDL_SetTextureColorMod(texture, sprite.color_mod.r, sprite.color_mod.g, sprite.color_mod.b);
            SDL_SetTextureAlphaMod(texture, sprite.color_mod.a);

            SDL_FRect source_rect = sprite.source_rect;
            if (auto* anim = registry.try_get<const ecs::SpriteAnimationComponent>(entity))
            {
                if (!anim->frames.empty())
                {
                    source_rect = anim->frames[anim->current_frame];
                }
            }

            bool use_entire_texture = (source_rect.w < 1e-6f || source_rect.h < 1e-6f);

            SDL_RenderTextureRotated(renderer, texture, use_entire_texture ? nullptr : &source_rect, &transform.destination_rect, transform.rotation, &transform.center, SDL_FLIP_NONE);
        }
    }

    void SetParent(entt::registry& registry, entt::entity child, entt::entity parent)
    {
        if (registry.all_of<ecs::ParentComponent>(child))
        {
            auto old_parent = registry.get<ecs::ParentComponent>(child).parent;

            if (registry.valid(old_parent) && registry.all_of<ecs::ChildrenComponent>(old_parent))
            {
                auto& children_comp = registry.get<ecs::ChildrenComponent>(old_parent);
                std::erase(children_comp.children, child);
            }
        }
        registry.emplace_or_replace<ecs::ParentComponent>(child, parent);
        registry.get_or_emplace<ecs::ChildrenComponent>(parent).children.push_back(child);
    }

    void AnimationSystem(entt::registry& registry, float delta_time)
    {
        auto view = registry.view<SpriteAnimationComponent, const VisibleComponent>();
        view.each([&](entt::entity, SpriteAnimationComponent& anime)
        {
            if (!anime.is_playing || anime.frames.empty() || anime.frame_duration <= 0.0f)
                return;

            anime.current_time += delta_time;
            if (anime.current_time >= anime.frame_duration)
            {
                anime.current_time = 0.0f;
                anime.current_frame++;
                if (anime.current_frame >= static_cast<int>(anime.frames.size()))
                {
                    if (anime.loop)
                        anime.current_frame = 0;
                    else
                    {
                        anime.current_frame = static_cast<int>(anime.frames.size()) - 1;
                        anime.is_playing = false;
                    }
                }
            }
        });
    }

    void TweenSystem(entt::registry& registry, float delta_time)
    {
        auto view = registry.view<ecs::TweenListComponent>();
        for (auto entity : view)
        {
            auto& tween_list = view.get<ecs::TweenListComponent>(entity);
            for (auto it = tween_list.tweens.begin(); it != tween_list.tweens.end();)
            {
                auto& tween = *it;
                if (tween.is_finished)
                {
                    ++it;
                    continue;
                }

                tween.elapsed_time += delta_time;
                float progress = std::min(1.0f, tween.elapsed_time / tween.duration);

                float eased_progress = progress;
                switch (tween.easing_type)
                {
                case ecs::Tween::EaseType::EASE_IN_QUAD:    eased_progress = Easing::EaseInQuad(progress); break;
                case ecs::Tween::EaseType::EASE_OUT_QUAD:   eased_progress = Easing::EaseOutQuad(progress); break;
                case ecs::Tween::EaseType::EASE_IN_OUT_QUAD:eased_progress = Easing::EaseInOutQuad(progress); break;
                case ecs::Tween::EaseType::EASE_OUT_SINE:   eased_progress = Easing::EaseOutSine(progress); break;
                case ecs::Tween::EaseType::LINEAR:
                default:                                    eased_progress = Easing::Linear(progress); break;
                }

                float current_value = tween.start_value + (tween.end_value - tween.start_value) * eased_progress;

                switch (tween.target_property)
                {
                case ecs::Tween::Property::TRANSFORM_POSITION_X:
                    if (auto* transform = registry.try_get<ecs::LocalTransformComponent>(entity))
                    {
                        transform->position.x = current_value;
                    }
                    break;
                case ecs::Tween::Property::TRANSFORM_POSITION_Y:
                    if (auto* transform = registry.try_get<ecs::LocalTransformComponent>(entity))
                    {
                        transform->position.y = current_value;
                    }
                    break;
                case ecs::Tween::Property::SPRITE_ALPHA:
                    if (auto* sprite = registry.try_get<ecs::SpriteComponent>(entity))
                    {
                        sprite->color_mod.a = static_cast<Uint8>(std::clamp(current_value, 0.0f, 255.0f));
                    }
                    break;
                }

                if (progress >= 1.0f)
                {
                    tween.is_finished = true;
                }
                ++it;
            }

            std::erase_if(tween_list.tweens, [](const auto& t) { return t.is_finished; });
        }
    }

    void CommandSystem(entt::registry& registry, runtime::ecs::Scene& scene, runtime::GameStateManager& gsm, platform::EventBus& bus)
    {
        CommandQueue& queue = scene.getCommandQueue();
        while (!queue.empty())
        {
            CommandPacket packet = std::move(queue.front());
            queue.pop_front();
            if (packet.command)
            {
                packet.command->execute(registry, gsm, bus, packet.source_entity);
            }
        }
    }

    void InteractionSystem::pushCommands(const std::vector<std::shared_ptr<commands::ICommand>>& commands, entt::entity source) const
    {
        for (const auto& cmd_ptr : commands)
        {
            if (cmd_ptr)
            {
                command_queue_->push_back({ .command = cmd_ptr->clone(), .source_entity = source });
            }
        }
    }

    InteractionSystem::InteractionSystem(const std::shared_ptr<entt::registry>& registry, platform::EventBus& bus, const std::shared_ptr<cyanvne::ecs::CommandQueue>& command_queue)
        : registry_(registry), command_queue_(command_queue)
    {
        bus.subscribeSDL([this](const SDL_Event& event) {
            return processInput(event);
        });
    }

    bool InteractionSystem::processInput(const SDL_Event& event) const
    {
        if (event.type == SDL_EVENT_MOUSE_BUTTON_UP)
        {
            float mouse_x = (float)event.button.x;
            float mouse_y = (float)event.button.y;

            auto view = registry_->view<const ClickableComponent, const FinalTransformComponent, const VisibleComponent>();
            bool processed = false;

            view.each([this, &event, mouse_x, mouse_y, &processed](entt::entity entity, const ClickableComponent& clickable, const FinalTransformComponent& transform) {
                SDL_FPoint point = { mouse_x, mouse_y };
                if (SDL_PointInRectFloat(&point, &transform.destination_rect))
                {
                    if (event.button.button == SDL_BUTTON_LEFT) { pushCommands(clickable.on_left_click, entity); }
                    else if (event.button.button == SDL_BUTTON_RIGHT) { pushCommands(clickable.on_right_click, entity); }
                    processed = true;
                }
            });

            if (processed)
                return true;
        }
        else if (event.type == SDL_EVENT_KEY_DOWN)
        {
            auto view = registry_->view<const KeyFocusComponent, const HasKeyFocus>();
            bool processed = false;

            view.each([this, &event, &processed](entt::entity entity, const KeyFocusComponent& key_focus) {
                if (auto it = key_focus.key_actions.find(event.key.key); it != key_focus.key_actions.end())
                {
                    pushCommands(it->second, entity);
                    processed = true;
                }
            });

            if (processed)
                return true;
        }
        else if (event.type == SDL_EVENT_MOUSE_WHEEL)
        {
            float mouse_x, mouse_y;
            SDL_GetMouseState(&mouse_x, &mouse_y);

            auto view = registry_->view<const ScrollableComponent, const FinalTransformComponent, const VisibleComponent>();
            bool processed = false;

            view.each([this, &event, mouse_x, mouse_y, &processed](entt::entity entity, const ScrollableComponent& scrollable, const FinalTransformComponent& transform)
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
            if (processed) return true;
        }
        return false;
    }
}