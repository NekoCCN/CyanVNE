#pragma once
#include <Runtime/ICommand/ICommand.h>
#include <Platform/EventBus/EventBus.h>
#include <Runtime/GameStateManager/GameStateManager.h>
#include <Runtime/RuntimeException/RuntimeException.h>
#include <Platform/ErrorEvent/ErrorEvent.h>
#include <entt/entt.hpp>

namespace cyanvne
{
	namespace runtime
	{
		namespace commands
		{
            template<std::copy_constructible EventT>
            class PublishEventCommand : public ecs::commands::ICommand
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

		    class ChangeIdentifierComponent : public ecs::commands::ICommand
		    {
		    private:
		    	std::string old_id_;
		    	std::string new_id_;
		    	bool is_error_fatal_;
		    public:
		    	ChangeIdentifierComponent(std::string old_id, std::string new_id, bool is_error_fatal = true)
		    		: old_id_(std::move(old_id)), new_id_(std::move(new_id)), is_error_fatal_(is_error_fatal)
		    	{  }

		    	void execute(entt::registry& registry, runtime::GameStateManager&, platform::EventBus& bus, entt::entity) const
		    	{
		    		auto target_view = registry.view<ecs::IdentifierComponent>();

		    		for (auto entity : target_view)
		    		{
		    			auto& identifier = target_view.get<ecs::IdentifierComponent>(entity);
		    			if (identifier.id == old_id_)
		    			{
		    				identifier.id = new_id_;
		    				return;
		    			}
		    		}
		    		if (is_error_fatal_)
		    		{
		    			throw exception::runtimeexception::CyanvneCommandExecuteException(
		    				"ChangeIdentifierComponent",
		    				std::format("No entity with identifier '{}' found.", old_id_));
		    		}
		    		else
		    		{
		    			bus.publish(
		    				platform::events::ErrorEvent(
		    					std::format("No entity with identifier '{}' found.", old_id_),
		    					"ChangeIdentifierComponent",
		    					false
		    				)
		    			);
		    		}
		    	}
		    };

			class ChangeLayoutComponent : public ecs::commands::ICommand
			{
			private:
				std::string identifier_;
				SDL_FRect new_area_ratio_;
				SDL_FPoint new_anchor_;
				bool is_error_fatal_;
			public:
				ChangeLayoutComponent(std::string identifier, SDL_FRect new_area_ratio, SDL_FPoint new_anchor, bool is_error_fatal = true)
					: identifier_(std::move(identifier)), new_area_ratio_(new_area_ratio), new_anchor_(new_anchor), is_error_fatal_(is_error_fatal)
				{  }
				void execute(entt::registry& registry, runtime::GameStateManager&, platform::EventBus& bus, entt::entity) const
				{
					auto target_view = registry.view<ecs::IdentifierComponent, ecs::LayoutComponent>();
					bool success = false;

					target_view.each([&](const entt::entity& entity, const ecs::IdentifierComponent& identifier, ecs::LayoutComponent& layout)
					{
						if (identifier.id == identifier_)
						{
							auto& layout = target_view.get<ecs::LayoutComponent>(entity);
							layout.area_ratio = new_area_ratio_;
							layout.anchor = new_anchor_;

							success = true;
							return;
						}
					});

					if (!success)
					{
						if (is_error_fatal_)
						{
							throw exception::runtimeexception::CyanvneCommandExecuteException(
								"ChangeLayoutComponent",
								std::format("No entity with identifier '{}' found.", identifier_));
						}
						else
						{
							bus.publish(
								platform::events::ErrorEvent(
									std::format("No entity with identifier '{}' found.", identifier_),
									"ChangeLayoutComponent",
									false
								)
							);
						}
					}
				}
			};

			class ChangeVisibleComponent : public ecs::commands::ICommand
			{
			private:
				std::string identifier_;
				bool is_visible_;
				bool is_error_fatal_;
			public:
				ChangeVisibleComponent(std::string identifier, bool is_visible, bool is_error_fatal = true)
					: identifier_(std::move(identifier)), is_visible_(is_visible), is_error_fatal_(is_error_fatal)
				{  }
				void execute(entt::registry& registry, runtime::GameStateManager&, platform::EventBus& bus, entt::entity) const
				{
					auto target_view = registry.view<ecs::IdentifierComponent>();

					for (auto& entity : target_view)
					{
						auto& identifier = target_view.get<ecs::IdentifierComponent>(entity);
						if (identifier.id == identifier_)
						{
							if (is_visible_)
							{
								if (!registry.any_of<ecs::VisibleComponent>(entity))
								{
									registry.emplace<ecs::VisibleComponent>(entity);
								}
							}
							else
							{
								if (registry.any_of<ecs::VisibleComponent>(entity))
								{
									registry.remove<ecs::VisibleComponent>(entity);
								}
							}

							return;
						}

						if (is_error_fatal_)
						{
							throw exception::runtimeexception::CyanvneCommandExecuteException(
								"ChangeVisibleComponent",
								std::format("No entity with identifier '{}' found.", identifier_));
						}
						else
						{
							bus.publish(
								platform::events::ErrorEvent(
									std::format("No entity with identifier '{}' found.", identifier_),
									"ChangeVisibleComponent",
									false
								)
							);
						}
					}
				}
			};

			class ChangeTransformComponent : public ecs::commands::ICommand
			{
			private:
				std::string identifier_;
				SDL_FPoint new_position_;
				SDL_FPoint new_scale_;
				bool is_error_fatal_;
			public:
				ChangeTransformComponent(std::string identifier, SDL_FPoint new_position, SDL_FPoint new_scale, bool is_error_fatal = true)
					: identifier_(std::move(identifier)), new_position_(new_position), new_scale_(new_scale), is_error_fatal_(is_error_fatal)
				{  }
				void execute(entt::registry& registry, runtime::GameStateManager&, platform::EventBus& bus, entt::entity) const
				{
					auto target_view = registry.view<ecs::IdentifierComponent, ecs::TransformComponent>();

					for (const auto& entity : target_view)
					{
						auto& identifier = target_view.get<ecs::IdentifierComponent>(entity);
						if (identifier.id == identifier_)
						{
							auto& [position, scale] = target_view.get<ecs::TransformComponent>(entity);
							position = new_position_;
							scale = new_scale_;
							return;
						}
					}

					if (is_error_fatal_)
					{
						throw exception::runtimeexception::CyanvneCommandExecuteException(
							"ChangeTransformComponent",
							std::format("No entity with identifier '{}' found.", identifier_));
					}
					else
					{
						bus.publish(
							platform::events::ErrorEvent(
								std::format("No entity with identifier '{}' found.", identifier_),
								"ChangeTransformComponent",
								false
							)
						);
					}
				}
			};
		}
	}
}