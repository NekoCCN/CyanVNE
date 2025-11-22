#include "Systems.h"
#include "Runtime/Components/Components.h"
#include "Platform/Thread/UnifiedConcurrencyManager.h"
#include "Core/Logger/Logger.h"
#include <glm/gtc/type_ptr.hpp>

namespace cyanvne::ecs::systems
{
    namespace
    {
        struct TextureLoadResult
        {
            entt::entity target_entity;
            std::string resource_alias;
            bool is_pinned;
            std::optional<resources::PinnedResourceHandle> data_handle;
            std::optional<resources::ResourceHandle<resources::TextureResource>> resource_handle;
            std::exception_ptr exception;
        };
        using TextureLoadResultPtr = std::shared_ptr<TextureLoadResult>;

        glm::mat4 calculate_local_transform(const runtime::TransformComponent &transform)
        {
            glm::mat4 local = glm::translate(glm::mat4(1.0f), glm::vec3(transform.position, 0.0f));
            local = glm::rotate(local, transform.rotation, glm::vec3(0.0f, 0.0f, 1.0f));
            local = glm::scale(local, glm::vec3(transform.scale, 1.0f));
            return local;
        }

        void update_children_transforms(entt::registry &registry, entt::entity parent, const glm::mat4 &parent_transform)
        {
            auto hierarchy_view = registry.view<runtime::HierarchyComponent>();
            if (!hierarchy_view.contains(parent))
                return;

            auto current_child = hierarchy_view.get<runtime::HierarchyComponent>(parent).first_child;
            while (current_child != entt::null && registry.valid(current_child))
            {
                if (auto *transform = registry.try_get<runtime::TransformComponent>(current_child))
                {
                    auto &world_transform = registry.get_or_emplace<runtime::WorldTransformComponent>(current_child);
                    world_transform.transform = parent_transform * calculate_local_transform(*transform);

                    update_children_transforms(registry, current_child, world_transform.transform);
                }

                current_child = hierarchy_view.get<runtime::HierarchyComponent>(current_child).next_sibling;
            }
        }
    }

    void ResourceLoadingSystem(entt::registry &registry,
                               const std::shared_ptr<resources::UnifiedCacheManager> &cache_manager,
                               platform::concurrency::UnifiedConcurrencyManager &concurrency_manager)
    {
        auto view = registry.view<runtime::MaterialComponent>();
        for (auto entity: view)
        {
            auto &material = view.get<runtime::MaterialComponent>(entity);
            if (material.load_state == runtime::MaterialComponent::LoadState::Unloaded)
            {
                material.load_state = runtime::MaterialComponent::LoadState::Loading;
                auto alias = material.texture_atlas_alias;
                auto is_pinned = material.is_pinned;

                concurrency_manager.submit_worker_and_continue_on_main(
                        [cache_manager, entity, alias, is_pinned]() -> TextureLoadResultPtr
                        {
                            auto result = std::make_shared<TextureLoadResult>();
                            result->target_entity = entity;
                            result->resource_alias = alias;
                            result->is_pinned = is_pinned;
                            try
                            {
                                if (is_pinned)
                                {
                                    result->data_handle = cache_manager->getUncachedBuffer(alias);
                                } else
                                {
                                    result->resource_handle = cache_manager->get<resources::TextureResource>(alias);
                                }
                            } catch (...)
                            {
                                result->exception = std::current_exception();
                            }
                            return result;
                        },
                        [&registry](std::optional<TextureLoadResultPtr> result_ptr_opt, const std::exception_ptr &)
                        {
                            if (!result_ptr_opt || !*result_ptr_opt)
                                return;
                            auto &result = **result_ptr_opt;

                            if (!registry.valid(result.target_entity))
                                return;
                            auto &material = registry.get<runtime::MaterialComponent>(result.target_entity);

                            if (result.exception)
                            {
                                material.load_state = runtime::MaterialComponent::LoadState::Failed;
                                try
                                {
                                    if (result.exception)
                                    {
                                        std::rethrow_exception(result.exception);
                                    }
                                } catch (const std::exception &e)
                                {
                                    core::GlobalLogger::getCoreLogger()->error("Failed to load texture '{}': {}",
                                                                               result.resource_alias, e.what());
                                }
                                return;
                            }

                            if (result.is_pinned)
                            {
                                if (result.data_handle)
                                {
                                    material.resource_handle.emplace<runtime::PinnedTexture>(
                                            std::move(*result.data_handle));
                                    material.load_state = runtime::MaterialComponent::LoadState::Loaded;
                                } else
                                {
                                    material.load_state = runtime::MaterialComponent::LoadState::Failed;
                                }
                            } else
                            {
                                if (result.resource_handle)
                                {
                                    material.resource_handle.emplace<resources::ResourceHandle<resources::TextureResource>>(
                                            std::move(*result.resource_handle));
                                    material.load_state = runtime::MaterialComponent::LoadState::Loaded;
                                } else
                                {
                                    material.load_state = runtime::MaterialComponent::LoadState::Failed;
                                }
                            }
                        }
                );
            }
        }
    }

    void TransformSystem(entt::registry &registry)
    {
        auto view = registry.view<runtime::TransformComponent>();
        for (auto entity: view)
        {
            bool is_root = true;
            if (auto *hierarchy = registry.try_get<runtime::HierarchyComponent>(entity))
            {
                if (registry.valid(hierarchy->parent))
                {
                    is_root = false;
                }
            }

            if (is_root)
            {
                auto &transform = view.get<runtime::TransformComponent>(entity);
                auto &world_transform = registry.get_or_emplace<runtime::WorldTransformComponent>(entity);

                world_transform.transform = calculate_local_transform(transform);

                update_children_transforms(registry, entity, world_transform.transform);
            }
        }
    }

    void RenderSystem(entt::registry &registry, runtime::MeshBatchRenderer &renderer)
    {
        auto camera_view = registry.view<runtime::CameraComponent, runtime::WorldTransformComponent>();
        for (auto camera_entity : camera_view)
        {
            auto& camera = camera_view.get<runtime::CameraComponent>(camera_entity);
            auto& camera_transform = camera_view.get<runtime::WorldTransformComponent>(camera_entity);

            if (camera.target_window.expired())
            {
                continue;
            }
            auto window_ptr = camera.target_window.lock();

            bgfx::ViewId view_id = static_cast<bgfx::ViewId>(camera.render_layer);
            bgfx::setViewFrameBuffer(view_id, window_ptr->getFrameBufferHandle());

            int32_t win_width, win_height;
            window_ptr->getWindowSize(&win_width, &win_height);

            const uint16_t view_x = static_cast<uint16_t>(win_width * camera.viewport_rect.x);
            const uint16_t view_y = static_cast<uint16_t>(win_height * camera.viewport_rect.y);
            const uint16_t view_w = static_cast<uint16_t>(win_width * camera.viewport_rect.z);
            const uint16_t view_h = static_cast<uint16_t>(win_height * camera.viewport_rect.w);
            bgfx::setViewRect(view_id, view_x, view_y, view_w, view_h);

            glm::mat4 view_mat = glm::inverse(camera_transform.transform);
            bgfx::setViewTransform(view_id, glm::value_ptr(view_mat), glm::value_ptr(camera.projection_matrix));

            bgfx::setViewClear(view_id, BGFX_CLEAR_COLOR | BGFX_CLEAR_DEPTH, 0x303030ff, 1.0f, 0);
            bgfx::touch(view_id);

            renderer.begin(view_id);

            auto render_view = registry.view<runtime::WorldTransformComponent, runtime::MeshComponent, runtime::MaterialComponent>();
            for (auto entity: render_view)
            {
                auto& mesh = render_view.get<runtime::MeshComponent>(entity);

                if ((camera.culling_mask & mesh.layer_mask) == 0)
                {
                    continue;
                }

                auto& world_transform = render_view.get<runtime::WorldTransformComponent>(entity);
                auto& material = render_view.get<runtime::MaterialComponent>(entity);

                if (material.load_state == runtime::MaterialComponent::LoadState::Loaded)
                {
                    renderer.submit(world_transform.transform, mesh, material);
                }
            }

            renderer.end();
        }
    }
}