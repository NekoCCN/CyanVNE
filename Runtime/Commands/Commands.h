#pragma once
#include <Runtime/ICommand/ICommand.h>
#include <Platform/EventBus/EventBus.h>
#include <Runtime/GameStateManager/GameStateManager.h>
#include <entt/entt.hpp>

namespace cyanvne
{
	namespace runtime
	{
		namespace commands
		{
            template<std::copy_constructible EventT>
            class PublishEventCommand : public ICommand
            {
            private:
                EventT event_to_publish_;
            public:
                explicit PublishEventCommand(EventT event) : event_to_publish_(event)
                {  }
                PublishEventCommand(const PublishEventCommand& other) : event_to_publish_(other.event_to_publish_)
                {  }

                std::unique_ptr<ICommand> clone() const override
                {
                    return std::make_unique<PublishEventCommand<EventT>>(*this);
                }

                void execute(entt::registry&, runtime::GameStateManager&, platform::EventBus& bus, entt::entity) const override
                {
                    bus.publish(event_to_publish_);
                }
            };
		}
	}
}