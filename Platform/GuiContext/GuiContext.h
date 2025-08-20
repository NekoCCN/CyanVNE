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
#include <Platform/WindowContext/WindowContext.h>
#include <backends/imgui_impl_sdl3.h>
#include "Detail/imgui_impl_bgfx.h"
#include <algorithm>
#include <memory>
#include <mutex>
#include <set>

namespace cyanvne
{
    namespace platform
    {
        class GuiContext
        {
        private:
            inline static std::mutex _mutex;
            inline static volatile GuiContext* _instance;

            ImGuiIO* io;

            char* font_data_ = nullptr;
            uint64_t font_size_;

            float font_pixels_size_ = 0;

            static float calculateScreenScale(const int32_t& screen_width, const int32_t& screen_height, const float base_font_size = 30.0f)
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
            GuiContext(const std::shared_ptr<WindowContext>& window,
                const std::vector<uint8_t>& font_data = {},
                float size_pixels = 30.0f,
                const std::set<std::string>& extra_languages_support = {});
        public:
            GuiContext(const GuiContext&) = delete;
            GuiContext& operator=(const GuiContext&) = delete;
            GuiContext(GuiContext&&) = delete;
            GuiContext& operator=(GuiContext&&) = delete;

            static std::shared_ptr<GuiContext> create(const std::shared_ptr<WindowContext>& window,
                const std::vector<uint8_t>& font_data = {},
                float size_pixels = 30.0f,
                const std::set<std::string>& extra_languages_support = {})
            {
                if (_instance == nullptr)
                {
                    _mutex.lock();
                    if (_instance == nullptr)
                    {
                        _instance = new GuiContext(window, font_data, size_pixels, extra_languages_support);
                    }
                    _mutex.unlock();
                }
                return std::shared_ptr<GuiContext>(const_cast<GuiContext*>(_instance));
            }

            static void response(const SDL_Event* event)
            {
                ImGui_ImplSDL3_ProcessEvent(event);
            }

            ImGuiIO& getIOContext() const
            {
                return *io;
            }

            void setupImGuiStyle(bool dark_style, float alpha)
            {
                ImGuiStyle& style = ImGui::GetStyle();
                [[maybe_unused]] ImVec4* colors = style.Colors;

                style.WindowPadding.x = 14.0f;
                style.WindowPadding.y = 14.0f;
                style.FramePadding.x = 10.0f;
                style.FramePadding.y = 5.0f;
                style.ItemSpacing.x = 14.0f;
                style.ItemSpacing.y = 5.0f;
                style.ScrollbarSize = 16;
                style.GrabMinSize = 10.0f;

                style.WindowRounding = 12.0f;
                style.FrameRounding = 8.0f;
                style.ChildRounding = 12.0f;
                style.PopupRounding = 12.0f;
                style.ScrollbarRounding = 18.0f;
            }

            void whenChangedWindowSize(int new_w, int new_h)
            {
                ImGui::GetIO().FontGlobalScale = calculateScreenScale(static_cast<float>(new_w),
                    static_cast<float>(new_h), font_pixels_size_);
            }

            ~GuiContext()
            {
                // Notice : This code will clean up the memory pointed to by the font pointer
                ImGui_ImplSDL3_Shutdown();
                ImGui_Implbgfx_Shutdown();
                ImGui::DestroyContext();
            }
        };
    }
}