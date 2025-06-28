#pragma once
#include <memory>
#include <vector>
#include "Platform/EventBus/EventBus.h"
#include "Platform/WindowContext/WindowContext.h"
#include "Resources/ICacheResourcesManager/ICacheResourcesManager.h"
#include "Runtime/Components/Components.h"

namespace cyanvne::runtime
{
	class IGameState;

	class GameStateManager
    {
    private:
        std::vector<std::unique_ptr<IGameState>> state_stack_;
        std::shared_ptr<platform::WindowContext> window_context_;
        std::shared_ptr<platform::EventBus> event_bus_;
        std::shared_ptr<resources::ICacheResourcesManager> cache_manager_;
        ecs::CommandQueue command_queue_;
        bool running_ = true;

    public:
        GameStateManager(std::shared_ptr<platform::WindowContext> window_ctx, std::shared_ptr<platform::EventBus> event_bus);
        ~GameStateManager();

        GameStateManager(const GameStateManager&) = delete;
        GameStateManager& operator=(const GameStateManager&) = delete;

        void pushState(std::unique_ptr<IGameState> new_state);
        void popState();
        void changeState(std::unique_ptr<IGameState> new_state);
        void quit();

        void handleEvents();
        void update(float delta_time);
        void render();

        bool isRunning() const
        {
            return running_;
        }
        bool hasStates() const
        {
            return !state_stack_.empty();
        }

        std::shared_ptr<platform::WindowContext> getWindowContext()
        {
            return window_context_;
        }
        std::shared_ptr<platform::EventBus> getEventBus()
        {
            return event_bus_;
        }
        std::shared_ptr<resources::ICacheResourcesManager> getResourceManager()
        {
            return cache_manager_;
        }
        ecs::CommandQueue& getCommandQueue()
        {
            return command_queue_;
        }
    };
}
