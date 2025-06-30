#include "GuiSystem.h"
#include "Runtime/GameStateManager/GameStateManager.h"

cyanvne::runtime::ecs::systems::GuiSystem::GuiSystem(cyanvne::runtime::GameStateManager& gsm, cyanvne::platform::EventBus& event_bus) : gsm_(gsm)
{
    event_bus.subscribe<events::ToggleGuiWindowEvent>(
        [this](const events::ToggleGuiWindowEvent& event)
        {
            this->onToggleWindow(event);
            return true;
        }
    );
}

void cyanvne::runtime::ecs::systems::GuiSystem::registerWindow(const std::string& id, std::unique_ptr<IGuiWindow> window) {
    windows_[id] = std::move(window);
}

void cyanvne::runtime::ecs::systems::GuiSystem::onToggleWindow(const events::ToggleGuiWindowEvent& event) const
{
    if (windows_.contains(event.window_id))
    {
        windows_.at(event.window_id)->toggleVisibility();
    }
}
