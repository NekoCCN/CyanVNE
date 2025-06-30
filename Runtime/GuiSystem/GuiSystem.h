#pragma once
#include <string>
#include <unordered_map>
#include "Runtime/GameStateManager/GameStateManager.h"
#include <Runtime/IGuiWindow/IGuiWindow.h>
#include "Runtime/Events/Events.h"

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
                void onToggleWindow(const events::ToggleGuiWindowEvent& event) const;

                void render()
                {
                    for (auto& window : windows_ | std::views::values)
                        window->render(gsm_);
                }
            };
        }
    }
}
