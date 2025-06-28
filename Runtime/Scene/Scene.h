#pragma once
#include <string>
#include <memory>
#include "entt/entt.hpp"
#include <Resources/ICacheResourcesManager/ICacheResourcesManager.h>
#include <Platform/WindowContext/WindowContext.h>

namespace cyanvne::runtime::ecs
{
    class Scene
    {
    private:
        entt::registry registry_;

    public:
        Scene() = default;

        entt::entity createEntity(const std::string& id = "");
        void update(float delta_time);
        void render(platform::WindowContext& window, resources::ICacheResourcesManager& cache_manager);

        entt::registry& getRegistry()
        {
            return registry_;
        }
    };
