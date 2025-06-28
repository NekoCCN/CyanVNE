#pragma once
#include <imgui.h>
#include <Runtime/GameStateManager/GameStateManager.h>

namespace cyanvne::runtime
{
    class IGuiWindow
    {
    protected:
        bool is_open_ = false;
    public:
        virtual ~IGuiWindow() = default;
        void render(GameStateManager& gsm)
        {
            if (is_open_)
            {
                ImGuiIO& io = ImGui::GetIO();

                ImVec2 window_size = getWindowSize(io);

                ImGui::SetNextWindowSize(window_size, ImGuiCond_Appearing);
                ImGui::SetNextWindowPos(ImVec2(io.DisplaySize.x * 0.5f, io.DisplaySize.y * 0.5f),
                    ImGuiCond_Appearing, ImVec2(0.5f, 0.5f));

                if (ImGui::Begin(getWindowTitle(), &is_open_, getWindowFlags()))
                {
                    draw_content(gsm);
                }
                ImGui::End();
            }
        }
        void toggleVisibility()
        {
            is_open_ = !is_open_;
        }
    protected:
        virtual const char* getWindowTitle() const = 0;
        virtual ImGuiWindowFlags getWindowFlags() const { return ImGuiWindowFlags_None; }
        virtual ImVec2 getWindowSize(const ImGuiIO& io) const { return ImVec2(io.DisplaySize.x * 0.6f, io.DisplaySize.y * 0.5f); }
        virtual void draw_content(GameStateManager& gsm) = 0;
    };
}