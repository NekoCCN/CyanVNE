#include "Scene.h"
#include <Runtime/Systems/Systems.h>
#include <Runtime/Components/Components.h>

entt::entity cyanvne::runtime::ecs::Scene::createEntity(const std::string& id)
{
    auto entity = registry_.create();
    if (!id.empty())
    {
        registry_.emplace<cyanvne::ecs::IdentifierComponent>(entity, id);
    }
    return entity;
}

void cyanvne::runtime::ecs::Scene::update(float delta_time)
{
	cyanvne::ecs::systems::AnimationSystem(registry_, delta_time);
}


void cyanvne::runtime::ecs::Scene::render(platform::WindowContext& window, cyanvne::resources::ICacheResourcesManager& cache_manager)
{
	cyanvne::ecs::systems::LayoutSystem(registry_, window, cache_manager);

	cyanvne::ecs::systems::RenderSystem(registry_, window, cache_manager);
}
