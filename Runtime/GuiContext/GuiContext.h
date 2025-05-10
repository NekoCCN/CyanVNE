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
#include <Runtime/WindowContext/WindowContext.h>
#include <Runtime/BasicRender/BasicRender.h>
#include <algorithm>
#include <memory>
#include <mutex>

namespace cyanvne
{
    namespace runtime
    {
        class GuiContext : public basicrender::ChangeableWindowSizeInterface
        {
        private:
            static std::mutex _mutex;
            static volatile GuiContext* _instance;

            ImGuiIO* io;

            char* font_data_ = nullptr;
            uint64_t font_size_;

            float font_pixels_size_ = 0;

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
            GuiContext(const std::shared_ptr<WindowContext>& window, const std::shared_ptr<char>& font_data = nullptr,
                uint64_t font_size = 0, float size_pixels = 30.0f, float scale = 1.0f);
        public:
            GuiContext(const GuiContext&) = delete;
            GuiContext& operator=(const GuiContext&) = delete;
            GuiContext(GuiContext&&) = delete;
            GuiContext& operator=(GuiContext&&) = delete;

            static std::shared_ptr<GuiContext> create(const std::shared_ptr<WindowContext>& window, const std::shared_ptr<char>& font_data = nullptr,
                uint64_t font_size = 0, float size_pixels = 30.0f, float scale = 1.0f)
            {
                if (window == nullptr)
                {
                    _mutex.lock();
                    if (_instance == nullptr)
                    {
                        _instance = new GuiContext(window, font_data, font_size, size_pixels, scale);
                    }
                    _mutex.unlock();
                }
                return std::shared_ptr<GuiContext>(const_cast<GuiContext*>(_instance));
            }

            ImGuiIO& getIOContext() const
            {
                return *io;
            }

            void setupImGuiStyle(bool bStyleDark_, float alpha_)
            {

            }

            void whenChangedWindowSize(int new_w, int new_h) override
            {
                ImGui::GetIO().FontGlobalScale = calculateScreenScale(static_cast<float>(new_w), static_cast<float>(new_h), font_pixels_size_);
            }

            ~GuiContext() override
            {
                // Notice : This code will clean up the memory pointed to by the font pointer
                ImGui_ImplSDLRenderer3_Shutdown();
                ImGui_ImplSDL3_Shutdown();
                ImGui::DestroyContext();
            }
        };
    }
}