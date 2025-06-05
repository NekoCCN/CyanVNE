#include "GameStateManager.h"
#include <Core/Logger/Logger.h>

cyanvne::runtime::GameStateManager::~GameStateManager()
{
    core::GlobalLogger::getCoreLogger()->info("GameStateManager destroyed.");
    while (!state_stack_.empty())
    {
        state_stack_.back()->shutdown(*this);
        state_stack_.pop_back();
    }
}

void cyanvne::runtime::GameStateManager::pushState(std::unique_ptr<IGameState> new_state)
{
    if (!new_state)
    {
        return;
    }
    if (!state_stack_.empty())
    {
        state_stack_.back()->pause(*this);
    }
    new_state->init(*this);
    state_stack_.push_back(std::move(new_state));
}

void cyanvne::runtime::GameStateManager::popState()
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

void cyanvne::runtime::GameStateManager::changeState(std::unique_ptr<IGameState> new_state)
{
    while (!state_stack_.empty())
    {
        state_stack_.back()->shutdown(*this);
        state_stack_.pop_back();
    }
    pushState(std::move(new_state));
}

void cyanvne::runtime::GameStateManager::updateCurrentState(float deltaTime)
{
    if (!state_stack_.empty())
    {
        state_stack_.back()->update(*this, deltaTime);
    }
}

void cyanvne::runtime::GameStateManager::renderCurrentStates()
{
    for (const auto& state_ptr : state_stack_)
    {
        if (state_ptr)
        {
            state_ptr->render(*this);
        }
    }
}