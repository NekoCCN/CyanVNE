#pragma once
#include <memory>

namespace cyanvne::runtime
{
	class GameStateManager;
}

namespace cyanvne::ecs::commands
{
    class ICommand
    {
    public:
        virtual ~ICommand() = default;
        virtual void execute(
            entt::registry& registry,
            runtime::GameStateManager& gsm,
            platform::EventBus& bus,
            entt::entity source_entity
        ) const = 0;

        virtual std::unique_ptr<ICommand> clone() const = 0;
    };
}
