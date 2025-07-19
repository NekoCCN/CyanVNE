#pragma once
#include <string>
#include <memory>
#include <Resources/ICacheResourcesManager/ICacheResourcesManager.h>
#include <Platform/WindowContext/WindowContext.h>
#include "Runtime/Components/Components.h"
#include "Runtime/Systems/Systems.h"

namespace cyanvne::runtime::ecs
{
    class Scene
    {
    private:
        std::shared_ptr<entt::registry> registry_;
        std::shared_ptr<cyanvne::ecs::CommandQueue> command_queue_;

        cyanvne::ecs::systems::InteractionSystem interaction_system_;
    public:
        Scene(platform::EventBus& event_bus) : registry_(std::make_shared<entt::registry>()), command_queue_(std::make_shared<cyanvne::ecs::CommandQueue>()),
        interaction_system_(registry_, event_bus, command_queue_)
        {  }

        entt::entity createEntity(const std::string& id = "");
        void update(float delta_time, platform::EventBus& event_bus);
        void render(platform::WindowContext& window, resources::ICacheResourcesManager& cache_manager);

        entt::registry& getRegistry()
        {
            return *registry_;
        }

        cyanvne::ecs::CommandQueue& getCommandQueue()
        {
            return *command_queue_;
        }
    };
}
