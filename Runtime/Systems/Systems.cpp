#include "Systems.h"

namespace cyanvne::ecs::systems
{
    void LayoutSystem(entt::registry& registry, const platform::WindowContext& window, const resources::ICacheResourcesManager& cache_manager)
    {
        auto view = registry.view<LayoutComponent>();
        SDL_Rect window_rect = window.getWindowRect();

        auto window_w = static_cast<float>(window_rect.w);
        auto window_h = static_cast<float>(window_rect.h);

        for (auto entity : view)
        {
            auto& layout = view.get<LayoutComponent>(entity);
            SDL_FRect final_rect;
            float target_w_px = layout.area_ratio.w * window_w;
            float target_h_px = layout.area_ratio.h * window_h;

            if ((target_w_px <= 0.0f && target_h_px > 0.0f) || (target_w_px > 0.0f && target_h_px <= 0.0f))
            {
                if (auto* sprite = registry.try_get<SpriteComponent>(entity))
                {
                    float tex_w, tex_h;

                    SDL_Texture* texture = cache_manager.getTexture(sprite->resource_key).get()->texture;

                    if (SDL_GetTextureSize(texture, &tex_w, &tex_h) && tex_w > 0 && tex_h > 0)
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
            final_rect.w = target_w_px;
            final_rect.h = target_h_px;
            final_rect.x = (layout.area_ratio.x * window_w) - (final_rect.w * layout.anchor.x);
            final_rect.y = (layout.area_ratio.y * window_h) - (final_rect.h * layout.anchor.y);

            registry.emplace_or_replace<RenderTransformComponent>(entity, final_rect);
        }
    }

    void TransformSystem(entt::registry& registry, const platform::WindowContext& window)
    {
        auto view = registry.view<TransformComponent>();

        for (auto entity : view)
        {
            auto& transform = view.get<TransformComponent>(entity);
            RenderTransformComponent render_transform = {};

            render_transform.destination_rect.x = transform.position.x;
            render_transform.destination_rect.y = transform.position.y;
            render_transform.destination_rect.w = static_cast<float>(window.getWindowRect().w) * transform.scale.x;
            render_transform.destination_rect.h = static_cast<float>(window.getWindowRect().h) * transform.scale.y;

            registry.emplace_or_replace<RenderTransformComponent>(entity, render_transform);
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

    void RenderSystem(entt::registry& registry, const platform::WindowContext& window, const resources::ICacheResourcesManager& cache_manager)
    {
        auto view = registry.view<const RenderTransformComponent, const SpriteComponent, const VisibleComponent>();

        view.each([&](entt::entity entity, const RenderTransformComponent& transform, const SpriteComponent& sprite)
            {
                auto texture_handle = cache_manager.getTexture(sprite.resource_key);
                SDL_Texture* texture = texture_handle->texture;
                if (!texture)
                    return;

                SDL_FRect source_rect = sprite.source_rect;
                if (auto* anime = registry.try_get<const SpriteAnimationComponent>(entity))
                {
                    if (!anime->frames.empty())
                        source_rect = anime->frames[anime->current_frame];
                }

                bool use_entire_texture = (source_rect.w - 0 < std::abs(0e-6) || source_rect.h - 0 < std::abs(0e-6));

                SDL_RenderTexture(window.getRendererHinding(), texture, use_entire_texture ? nullptr : &source_rect, &transform.destination_rect);
            });
    }

    void CommandSystem(entt::registry& registry, runtime::GameStateManager& gsm, platform::EventBus& bus)
    {
        auto& queue = gsm.getCommandQueue();
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