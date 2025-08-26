#pragma once

#include <memory>
#include <Runtime/GameStateManager/GameStateManager.h>
#include <entt/entt.hpp>

namespace cyanvne::runtime
{
    class EcsGameState : public IGameState
    {
    protected:
        entt::registry registry_;
    public:
        EcsGameState();

        void init(std::shared_ptr<GameStateManager> manager) override;
        void shutdown(std::shared_ptr<GameStateManager> manager) override;

        void handle_events(std::shared_ptr<GameStateManager> manager) override;

        void update(std::shared_ptr<GameStateManager> manager, float delta_time) override;

        void render(std::shared_ptr<GameStateManager> manager) override;
    };
}