#pragma once
#include <memory>
#include <vector>
#include <Runtime/IGameState/IGameState.h>
#include <Platform/WindowContext/WindowContext.h>
#include <Platform/EventBus/EventBus.h>

namespace cyanvne
{
	namespace runtime
	{
        class GameStateManager
        {
        private:
            std::vector<std::unique_ptr<IGameState>> state_stack_;
            std::shared_ptr<platform::EventBus> event_bus_;
            
            std::shared_ptr<platform::WindowContext> window_context_;
        public:
            GameStateManager(const std::shared_ptr<platform::WindowContext>& window_context,
                const std::shared_ptr<platform::EventBus>& event_bus) :
                event_bus_(event_bus),
                window_context_(window_context)
            {
                state_stack_.reserve(10);
            }
            GameStateManager(const GameStateManager&) = delete;
            GameStateManager(GameStateManager&&) = delete;
            GameStateManager& operator=(const GameStateManager&) = delete;
            GameStateManager& operator=(GameStateManager&&) = delete;

            void pushState(std::unique_ptr<IGameState> new_state);
            void popState();
            void changeState(std::unique_ptr<IGameState> new_state);

            void updateCurrentState(float deltaTime);
            void renderCurrentStates();

            bool isStateAtTop(const IGameState* state_ptr) const
            {
                if (state_stack_.empty() || !state_ptr)
                {
                    return false;
                }

                return state_stack_.back().get() == state_ptr;
            }

            std::shared_ptr<platform::WindowContext>& getWindowContext()
            {
                return window_context_;
            }

            std::shared_ptr<cyanvne::platform::EventBus> getEventBus()
            {
                return event_bus_;
            }

            bool hasStates() const
            {
                return !state_stack_.empty();
            }

            ~GameStateManager();
        };
	}
}