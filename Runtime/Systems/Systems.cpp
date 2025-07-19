#include "Systems.h"
#include "Runtime/Scene/Scene.h"

namespace cyanvne::ecs::systems
{
    void LayoutSystem(entt::registry& registry, const cyanvne::platform::WindowContext& window, const cyanvne::resources::ICacheResourcesManager& cache_manager) {
        auto view = registry.view<ecs::LayoutComponent>();
        SDL_Rect window_rect = window.getWindowRect();

        auto window_w = static_cast<float>(window_rect.w);
        auto window_h = static_cast<float>(window_rect.h);

        for (auto entity : view)
        {
            const auto& layout = view.get<const ecs::LayoutComponent>(entity);
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
                        float tex_w, tex_h;
                        if (SDL_GetTextureSize(texture_handle.get()->texture, &tex_w, &tex_h) == 0 && tex_w > 0 && tex_h > 0)
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

            registry.emplace_or_replace<ecs::FinalTransformComponent>(entity, final_rect);
        }
    }

    void SetParent(entt::registry& registry, entt::entity child, entt::entity parent)
    {
        if (registry.all_of<ecs::ParentComponent>(child)) {
            auto old_parent = registry.get<ecs::ParentComponent>(child).parent;
            if (registry.valid(old_parent) && registry.all_of<ecs::ChildrenComponent>(old_parent)) {
                auto& children_comp = registry.get<ecs::ChildrenComponent>(old_parent);
                std::erase(children_comp.children, child);
            }
        }
        registry.emplace_or_replace<ecs::ParentComponent>(child, parent);
        registry.get_or_emplace<ecs::ChildrenComponent>(parent).children.push_back(child);
    }

    void HierarchySystem(entt::registry& registry, const cyanvne::resources::ICacheResourcesManager& cache_manager)
    {
        auto root_view = registry.view<ecs::TransformComponent>(entt::exclude<ecs::ParentComponent>);
        for (auto entity : root_view)
        {
            const auto& root_transform = registry.get<ecs::TransformComponent>(entity);
            if (auto* children_comp = registry.try_get<ecs::ChildrenComponent>(entity))
            {
                for (auto child : children_comp->children)
                {
                    UpdateWorldTransformRecursive(registry, child, root_transform);
                }
            }
        }

        auto world_object_view = registry.view<ecs::TransformComponent>();
        for(auto entity : world_object_view)
        {
            const auto& world_transform = world_object_view.get<const ecs::TransformComponent>(entity);
            SDL_FRect final_rect;
            final_rect.x = world_transform.position.x;
            final_rect.y = world_transform.position.y;

            float tex_w = 64.0f, tex_h = 64.0f;
            if (auto* sprite = registry.try_get<ecs::SpriteComponent>(entity))
            {
                auto texture_handle = cache_manager.getTexture(sprite->resource_key);
                if (texture_handle.get() && texture_handle.get()->texture)
                {
                    float w, h;
                    if (SDL_GetTextureSize(texture_handle.get()->texture, &w, &h) == 0)
                    {
                        tex_w = w;
                        tex_h = h;
                    }
                }
            }
            final_rect.w = tex_w * world_transform.scale.x;
            final_rect.h = tex_h * world_transform.scale.y;

            registry.emplace_or_replace<ecs::FinalTransformComponent>(entity, final_rect);
        }
    }

    void UpdateWorldTransformRecursive(entt::registry& registry, entt::entity entity, const ecs::TransformComponent& parent_world_transform)
    {
        const auto& local_transform = registry.get<ecs::TransformComponent>(entity);
        auto& final_world_transform = registry.get<ecs::TransformComponent>(entity);
        final_world_transform.scale.x = parent_world_transform.scale.x * local_transform.scale.x;
        final_world_transform.scale.y = parent_world_transform.scale.y * local_transform.scale.y;
        final_world_transform.position.x = parent_world_transform.position.x + local_transform.position.x * parent_world_transform.scale.x;
        final_world_transform.position.y = parent_world_transform.position.y + local_transform.position.y * parent_world_transform.scale.y;
        final_world_transform.rotation = parent_world_transform.rotation + local_transform.rotation;

        if (auto* children_comp = registry.try_get<ecs::ChildrenComponent>(entity))
        {
            for (auto child : children_comp->children)
            {
                UpdateWorldTransformRecursive(registry, child, final_world_transform);
            }
        }
    }

    void AnimationSystem(entt::registry& registry, float delta_time)
    {
        auto view = registry.view<SpriteAnimationComponent, const VisibleComponent>();

        view.each([&](entt::entity entity, SpriteAnimationComponent& anime)
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

            for (auto& tween : tween_list.tweens)
            {
                if (tween.is_finished)
                    continue;

                tween.elapsed_time += delta_time;
                float progress = std::min(tween.elapsed_time / tween.duration, 1.0f);

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
                    if (auto* transform = registry.try_get<ecs::TransformComponent>(entity))
                    {
                        transform->position.x = current_value;
                    }
                    break;
                case ecs::Tween::Property::TRANSFORM_POSITION_Y:
                    if (auto* transform = registry.try_get<ecs::TransformComponent>(entity))
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
            }

            std::erase_if(tween_list.tweens, [](const auto& t) { return t.is_finished; });
        }
    }

    void RenderSystem(entt::registry& registry, const cyanvne::platform::WindowContext& window, const cyanvne::resources::ICacheResourcesManager& cache_manager)
    {
        SDL_Renderer* renderer = window.getRendererHinding();
        registry.sort<ecs::SpriteComponent>([](const auto& lhs, const auto& rhs)
        {
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
                    source_rect = anim->frames[anim->current_frame];
            }
            bool use_entire_texture = (source_rect.w < 1e-6f || source_rect.h < 1e-6f);
            SDL_RenderTexture(renderer, texture, use_entire_texture ? nullptr : &source_rect, &transform.destination_rect);
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
}
