#include "imgui.h"
#include "GuiSystem.h"

#include "Runtime/Components/Components.h"
#include "Runtime/GameStateManager/GameStateManager.h"

namespace cyanvne::runtime::ecs::systems
{
    GuiSystem::GuiSystem(entt::registry& registry, GameStateManager& gsm)
        : registry_(registry), gsm_(gsm)
    {
        window_visibility_["entity_inspector"] = true;
    }

    void GuiSystem::toggleWindowVisibility(const std::string& window_id)
    {
        window_visibility_[window_id] = !window_visibility_[window_id];
    }

    void GuiSystem::render(platform::WindowContext& window)
    {
        if (window_visibility_["demo_window"])
        {
            ImGui::ShowDemoWindow(&window_visibility_["demo_window"]);
        }
        if (window_visibility_["entity_inspector"])
        {
            renderEntityInspector();
        }
    }

    void GuiSystem::renderEntityInspector()
    {
        ImGui::Begin("实体观察器 (ECS Inspector)", &window_visibility_["entity_inspector"]);

        for (auto& entity : registry_.view)
        {
            std::string label = "实体 " + std::to_string(static_cast<uint32_t>(entity));
            if (auto* id = registry_.try_get<cyanvne::ecs::IdentifierComponent>(entity))
            {
                label += " (" + id->id + ")";
            }

            if (ImGui::TreeNode(label.c_str()))
            {
                if (auto* layout = registry_.try_get<cyanvne::ecs::LayoutComponent>(entity))
                {
                    if (ImGui::TreeNode("LayoutComponent"))
                    {
                        ImGui::DragFloat2("相对位置 (X, Y)", &layout->area_ratio.x, 0.01f);
                        ImGui::DragFloat2("相对大小 (W, H)", &layout->area_ratio.w, 0.01f);
                        ImGui::TreePop();
                    }
                }

                bool is_visible = registry_.all_of<cyanvne::ecs::VisibleComponent>(entity);
                if (ImGui::Checkbox("可见 (Visible)", &is_visible))
                {
                    if (is_visible)
                    {
                        registry_.emplace_or_replace<cyanvne::ecs::VisibleComponent>(entity);
                    }
                    else
                    {
                        registry_.remove<cyanvne::ecs::VisibleComponent>(entity);
                    }
                }

                ImGui::TreePop();
            }
        }

        ImGui::End();
    }
}
