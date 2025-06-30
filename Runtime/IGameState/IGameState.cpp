#include "IGameState.h"
#include <Runtime/Scene/Scene.h>

cyanvne::runtime::EcsGameState::EcsGameState() : scene_(std::make_unique<ecs::Scene>())
{  }

void cyanvne::runtime::EcsGameState::init(cyanvne::runtime::GameStateManager& manager)
{  }

void cyanvne::runtime::EcsGameState::shutdown(cyanvne::runtime::GameStateManager& manager)
{
    scene_.reset();
}

void cyanvne::runtime::EcsGameState::handle_events(cyanvne::runtime::GameStateManager& manager)
{
}

void cyanvne::runtime::EcsGameState::update(cyanvne::runtime::GameStateManager& manager, float delta_time)
{
    if (scene_)
        scene_->update(delta_time);
}

void cyanvne::runtime::EcsGameState::render(cyanvne::runtime::GameStateManager& manager)
{
    if (scene_)
        scene_->render(*manager.getWindowContext(), *manager.getResourceManager());
}