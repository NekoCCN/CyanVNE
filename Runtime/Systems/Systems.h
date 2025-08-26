#pragma once

#define M_PI       3.14159265358979323846
#include "Runtime/Components/Components.h"
#include "Resources/UnifiedCacheManager/UnifiedCacheManager.h"
#include <entt/entt.hpp>
#include <SDL3/SDL.h>

namespace cyanvne::ecs::systems
{
    void ResourceLoadingSystem(entt::registry& registry,
                               resources::UnifiedCacheManager& cache_manager,
                               platform::concurrency::UnifiedConcurrencyManager& concurrency_manager);
}