#include "EcsGameState.h"

cyanvne::runtime::EcsGameState::EcsGameState() : scene_(std::make_unique<ecs::Scene>())
{  }

void cyanvne::runtime::EcsGameState::init(std::shared_ptr<GameStateManager> manager)
{  }

void cyanvne::runtime::EcsGameState::shutdown(std::shared_ptr<GameStateManager> manager)
{
}

void cyanvne::runtime::EcsGameState::handle_events(std::shared_ptr<GameStateManager> manager)
{
}

void cyanvne::runtime::EcsGameState::update(std::shared_ptr<GameStateManager> manager, float delta_time)
{
}

void cyanvne::runtime::EcsGameState::render(std::shared_ptr<GameStateManager> manager)
{
}