#include "Systems.h"
#include "Runtime/Components/Components.h"
#include "Platform/Thread/UnifiedConcurrencyManager.h"
#include "Core/Logger/Logger.h"

namespace cyanvne::ecs::systems
{
    namespace {
        struct TextureLoadResult {
            entt::entity target_entity;
            std::string resource_alias;
            bool is_pinned;
            std::optional<resources::PinnedResourceHandle> data_handle;
            std::optional<resources::ResourceHandle<resources::TextureResource>> resource_handle;
            std::exception_ptr exception;
        };
        using TextureLoadResultPtr = std::shared_ptr<TextureLoadResult>;
    }

    void ResourceLoadingSystem(entt::registry &registry,
                               std::shared_ptr<resources::UnifiedCacheManager> cache_manager,
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
                            auto& result = **result_ptr_opt;

                            if (!registry.valid(result.target_entity))
                                return;
                            auto &material = registry.get<runtime::MaterialComponent>(result.target_entity);

                            if (result.exception)
                            {
                                material.load_state = runtime::MaterialComponent::LoadState::Failed;
                                try
                                {
                                    if(result.exception)
                                    {
                                        std::rethrow_exception(result.exception);
                                    }
                                } catch(const std::exception& e)
                                {
                                    core::GlobalLogger::getCoreLogger()->error("Failed to load texture '{}': {}", result.resource_alias, e.what());
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
}