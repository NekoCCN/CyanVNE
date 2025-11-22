#pragma once

#define M_PI       3.14159265358979323846
#include "Runtime/Components/Components.h"
#include "Resources/UnifiedCacheManager/UnifiedCacheManager.h"
#include <entt/entt.hpp>
#include <SDL3/SDL.h>

namespace cyanvne::ecs::systems
{
    void TransformSystem(entt::registry& registry);

    void RenderSystem(entt::registry& registry, runtime::MeshBatchRenderer& renderer);

    void ResourceLoadingSystem(entt::registry& registry,
                               const std::shared_ptr<resources::UnifiedCacheManager>& cache_manager,
                               platform::concurrency::UnifiedConcurrencyManager& concurrency_manager);
}