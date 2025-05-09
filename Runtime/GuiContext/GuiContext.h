#pragma once
#include <SDL3/SDL.h>
#include <imgui.h>
#include <backends/imgui_impl_sdl3.h>
#include <backends/imgui_impl_sdlrenderer3.h>
#if defined(IMGUI_IMPL_OPENGL_ES2)
#include <SDL3/SDL_opengles2.h>
#else
#include <SDL3/SDL_opengl.h>
#endif
#include "../Window/Window.h"
#include <memory>

namespace vn
{
    namespace core
    {
        class GuiContext
        {
        private:
            ImGuiIO* io;

            char* font_data_ = nullptr;
            uint64_t font_size_;

            static float calculateScreenScale(const float screen_width, const float screen_height, const float base_font_size = 30.0f)
            {
                constexpr float reference_width = 1920.0f;
                constexpr float reference_height = 1080.0f;

                float screen_diagonal = sqrt(screen_width * screen_width + screen_height * screen_height);
                float reference_diagonal = sqrt(reference_width * reference_width + reference_height * reference_height);

                float scale = screen_diagonal / reference_diagonal;

                float final_scale = (scale * base_font_size) / 30.0f;

                constexpr float min_scale = 0.5f;
                constexpr float max_scale = 2.0f;
                return std::clamp(final_scale, min_scale, max_scale);
            }

        public:
            GuiContext(const std::shared_ptr<core::Window>& window, const std::shared_ptr<char>& font_data = nullptr,
                uint64_t font_size = 0, float size_pixels = 30.0f, float scale = 1.0f)
                : font_size_(font_size)
            {
                SDL_LogInfo(SDL_LOG_CATEGORY_APPLICATION, "Try to Creating GUI Context");
                IMGUI_CHECKVERSION();
                ImGui::CreateContext();
                io = &ImGui::GetIO();
                io->ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;     // Enable Keyboard Controls
                io->ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;      // Enable Gamepad Controls
                io->ConfigFlags |= ImGuiConfigFlags_DockingEnable;         // Enable Docking

                SDL_LogInfo(SDL_LOG_CATEGORY_APPLICATION, "GUI Theme : Light");
                ImGui::StyleColorsLight();

                ImGui_ImplSDL3_InitForSDLRenderer(window->getWindowHinding(), window->getRendererHinding());
                ImGui_ImplSDLRenderer3_Init(window->getRendererHinding());


                if (font_data != nullptr && font_size_ > 0)
                {
                    font_data_ = new char[font_size];
                    memcpy(font_data_, font_data.get(), font_size);
                    SDL_LogInfo(SDL_LOG_CATEGORY_APPLICATION, "Try to Load Font");
                    io->Fonts->AddFontFromMemoryTTF(font_data_, font_size_, size_pixels);
                    ImGui::GetIO().FontGlobalScale = calculateScreenScale(window->getWindowRect().w, window->getWindowRect().h, size_pixels);
                }

                setupImGuiStyle(false, 0.7);
            }
            ImGuiIO& getIOContext() const
            {
                return *io;
            }

            static void setupImGuiStyle(bool bStyleDark_, float alpha_)
            {
                ImGuiStyle& style = ImGui::GetStyle();

                // light style from Pacôme Danhiez (user itamago) https://github.com/ocornut/imgui/pull/511#issuecomment-175719267
                style.Alpha = 1.0f;
                style.FrameRounding = 3.0f;
                style.Colors[ImGuiCol_Text] = ImVec4(0.00f, 0.00f, 0.00f, 1.00f);
                style.Colors[ImGuiCol_TextDisabled] = ImVec4(0.60f, 0.60f, 0.60f, 1.00f);
                style.Colors[ImGuiCol_WindowBg] = ImVec4(0.94f, 0.94f, 0.94f, 0.94f);
                style.Colors[ImGuiCol_ChildBg] = ImVec4(0.00f, 0.00f, 0.00f, 0.00f);
                style.Colors[ImGuiCol_PopupBg] = ImVec4(1.00f, 1.00f, 1.00f, 0.94f);
                style.Colors[ImGuiCol_Border] = ImVec4(0.00f, 0.00f, 0.00f, 0.39f);
                style.Colors[ImGuiCol_BorderShadow] = ImVec4(1.00f, 1.00f, 1.00f, 0.10f);
                style.Colors[ImGuiCol_FrameBg] = ImVec4(1.00f, 1.00f, 1.00f, 0.94f);
                style.Colors[ImGuiCol_FrameBgHovered] = ImVec4(0.26f, 0.59f, 0.98f, 0.40f);
                style.Colors[ImGuiCol_FrameBgActive] = ImVec4(0.26f, 0.59f, 0.98f, 0.67f);
                style.Colors[ImGuiCol_TitleBg] = ImVec4(0.96f, 0.96f, 0.96f, 1.00f);
                style.Colors[ImGuiCol_TitleBgCollapsed] = ImVec4(1.00f, 1.00f, 1.00f, 0.51f);
                style.Colors[ImGuiCol_TitleBgActive] = ImVec4(0.82f, 0.82f, 0.82f, 1.00f);
                style.Colors[ImGuiCol_MenuBarBg] = ImVec4(0.86f, 0.86f, 0.86f, 1.00f);
                style.Colors[ImGuiCol_ScrollbarBg] = ImVec4(0.98f, 0.98f, 0.98f, 0.53f);
                style.Colors[ImGuiCol_ScrollbarGrab] = ImVec4(0.69f, 0.69f, 0.69f, 1.00f);
                style.Colors[ImGuiCol_ScrollbarGrabHovered] = ImVec4(0.59f, 0.59f, 0.59f, 1.00f);
                style.Colors[ImGuiCol_ScrollbarGrabActive] = ImVec4(0.49f, 0.49f, 0.49f, 1.00f);
                //style.Colors[ImGuiCol_ComboBg] = ImVec4(0.86f, 0.86f, 0.86f, 0.99f);
                style.Colors[ImGuiCol_CheckMark] = ImVec4(0.26f, 0.59f, 0.98f, 1.00f);
                style.Colors[ImGuiCol_SliderGrab] = ImVec4(0.24f, 0.52f, 0.88f, 1.00f);
                style.Colors[ImGuiCol_SliderGrabActive] = ImVec4(0.26f, 0.59f, 0.98f, 1.00f);
                style.Colors[ImGuiCol_Button] = ImVec4(0.26f, 0.59f, 0.98f, 0.40f);
                style.Colors[ImGuiCol_ButtonHovered] = ImVec4(0.26f, 0.59f, 0.98f, 1.00f);
                style.Colors[ImGuiCol_ButtonActive] = ImVec4(0.06f, 0.53f, 0.98f, 1.00f);
                style.Colors[ImGuiCol_Header] = ImVec4(0.26f, 0.59f, 0.98f, 0.31f);
                style.Colors[ImGuiCol_HeaderHovered] = ImVec4(0.26f, 0.59f, 0.98f, 0.80f);
                style.Colors[ImGuiCol_HeaderActive] = ImVec4(0.26f, 0.59f, 0.98f, 1.00f);
                //style.Colors[ImGuiCol_Column] = ImVec4(0.39f, 0.39f, 0.39f, 1.00f);
                //style.Colors[ImGuiCol_ColumnHovered] = ImVec4(0.26f, 0.59f, 0.98f, 0.78f);
                //style.Colors[ImGuiCol_ColumnActive] = ImVec4(0.26f, 0.59f, 0.98f, 1.00f);
                style.Colors[ImGuiCol_ResizeGrip] = ImVec4(1.00f, 1.00f, 1.00f, 0.50f);
                style.Colors[ImGuiCol_ResizeGripHovered] = ImVec4(0.26f, 0.59f, 0.98f, 0.67f);
                style.Colors[ImGuiCol_ResizeGripActive] = ImVec4(0.26f, 0.59f, 0.98f, 0.95f);
                //style.Colors[ImGuiCol_CloseButton] = ImVec4(0.59f, 0.59f, 0.59f, 0.50f);
                //style.Colors[ImGuiCol_CloseButtonHovered] = ImVec4(0.98f, 0.39f, 0.36f, 1.00f);
                //style.Colors[ImGuiCol_CloseButtonActive] = ImVec4(0.98f, 0.39f, 0.36f, 1.00f);
                style.Colors[ImGuiCol_PlotLines] = ImVec4(0.39f, 0.39f, 0.39f, 1.00f);
                style.Colors[ImGuiCol_PlotLinesHovered] = ImVec4(1.00f, 0.43f, 0.35f, 1.00f);
                style.Colors[ImGuiCol_PlotHistogram] = ImVec4(0.90f, 0.70f, 0.00f, 1.00f);
                style.Colors[ImGuiCol_PlotHistogramHovered] = ImVec4(1.00f, 0.60f, 0.00f, 1.00f);
                style.Colors[ImGuiCol_TextSelectedBg] = ImVec4(0.26f, 0.59f, 0.98f, 0.35f);
                //style.Colors[ImGuiCol_ModalWindowDarkening] = ImVec4(0.20f, 0.20f, 0.20f, 0.35f);

                if (bStyleDark_)
                {
                    for (int i = 0; i <= ImGuiCol_COUNT; i++)
                    {
                        ImVec4& col = style.Colors[i];
                        float H, S, V;
                        ImGui::ColorConvertRGBtoHSV(col.x, col.y, col.z, H, S, V);

                        if (S < 0.1f)
                        {
                            V = 1.0f - V;
                        }
                        ImGui::ColorConvertHSVtoRGB(H, S, V, col.x, col.y, col.z);
                        if (col.w < 1.00f)
                        {
                            col.w *= alpha_;
                        }
                    }
                }
                else
                {
                    for (int i = 0; i <= ImGuiCol_COUNT; i++)
                    {
                        ImVec4& col = style.Colors[i];
                        if (col.w < 1.00f)
                        {
                            col.x *= alpha_;
                            col.y *= alpha_;
                            col.z *= alpha_;
                            col.w *= alpha_;
                        }
                    }
                }
            }

            static void whenChangedWindowSize(const std::shared_ptr<core::Window>& window)
            {
                ImGui::GetIO().FontGlobalScale = calculateScreenScale(window->getWindowRect().w, window->getWindowRect().h);
            }
            ~GuiContext()
            {
                // Notice : This code will clean up the memory pointed to by the font pointer
                ImGui_ImplSDLRenderer3_Shutdown();
                ImGui_ImplSDL3_Shutdown();
                ImGui::DestroyContext();
            }
        };
    }
}
#endif // VISUALNOVEL_GUICONTEXT_H
