#include "GameStateManager.h"
#include <Runtime/IGameState/IGameState.h>

namespace cyanvne::runtime
{
    GameStateManager::GameStateManager(std::shared_ptr<platform::WindowContext> window_ctx, std::shared_ptr<platform::EventBus> event_bus)
        : window_context_(std::move(window_ctx)), event_bus_(std::move(event_bus))
    {
        state_stack_.reserve(10);
    }

    GameStateManager::~GameStateManager()
    {
        while (!state_stack_.empty())
        {
            state_stack_.back()->shutdown(*this);
            state_stack_.pop_back();
        }
    }
    
    void GameStateManager::quit()
    {
        running_ = false;
    }

    void GameStateManager::pushState(std::unique_ptr<IGameState> new_state)
    {
        if (!new_state)
            return;

        if (!state_stack_.empty())
        {
            state_stack_.back()->pause(*this);
        }

        new_state->init(*this);

        state_stack_.push_back(std::move(new_state));
    }

    void GameStateManager::popState()
    {
        if (!state_stack_.empty())
        {
            state_stack_.back()->shutdown(*this);

            state_stack_.pop_back();

            if (!state_stack_.empty())
            {
                state_stack_.back()->resume(*this);
            }
        }
    }

    void GameStateManager::changeState(std::unique_ptr<IGameState> new_state)
    {
        while (!state_stack_.empty())
        {
            state_stack_.back()->shutdown(*this);

            state_stack_.pop_back();
        }
        pushState(std::move(new_state));
    }

    void GameStateManager::handleEvents()
    {
        if (!state_stack_.empty())
        {
            state_stack_.back()->handle_events(*this);
        }
    }

    void GameStateManager::update(float delta_time)
    {
        if (!state_stack_.empty())
        {
            state_stack_.back()->update(*this, delta_time);
        }
    }

    void GameStateManager::render()
    {
        for (const auto& state : state_stack_)
        {
            state->render(*this);
        }
    }
}
