#pragma once
#include <string>
#include <unordered_map>
#include <entt/entt.hpp>
#include "Runtime/GameStateManager/GameStateManager.h"
#include <Runtime/IGuiWindow/IGuiWindow.h>

namespace cyanvne
{
    namespace runtime
    {
        namespace ecs::systems
        {
            class GuiSystem
            {
            private:
                GameStateManager& gsm_;
                std::unordered_map<std::string, std::unique_ptr<IGuiWindow>> windows_;
            public:
                GuiSystem(GameStateManager& gsm, platform::EventBus& event_bus);
                void registerWindow(const std::string& id, std::unique_ptr<IGuiWindow> window);
                void onToggleWindow(const ecs::events::ToggleGuiWindow& event);
                void Render(platform::WindowContext& window);
            };
        }
    }
}
