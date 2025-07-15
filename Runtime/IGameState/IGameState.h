#pragma once
#include <Runtime/GameStateManager/GameStateManager.h>

namespace cyanvne::runtime
{
    namespace ecs
    {
        class Scene;
    }

    class IGameState
    {
    protected:
        IGameState() = default;
    public:
        IGameState(const IGameState&) = delete;
        IGameState& operator=(const IGameState&) = delete;
        IGameState(IGameState&&) = delete;
        IGameState& operator=(IGameState&&) = delete;

        virtual void init(GameStateManager& manager) = 0;
        virtual void shutdown(GameStateManager& manager) = 0;

        virtual void handle_events(GameStateManager& manager) = 0;
        virtual void update(GameStateManager& manager, float delta_time) = 0;
        virtual void render(GameStateManager& manager) = 0;

        virtual void pause(GameStateManager& manager)
        {  }
        virtual void resume(GameStateManager& manager)
        {  }

        virtual ~IGameState() = default;
    };

    class EcsGameState : public IGameState
    {
    protected:
        std::unique_ptr<ecs::Scene> scene_;
    public:
        EcsGameState();
        void init(GameStateManager& manager) override;
        void shutdown(GameStateManager& manager) override;
        void handle_events(GameStateManager& manager) override;
        void update(GameStateManager& manager, float delta_time) override;
        void render(GameStateManager& manager) override;
    };
}