#include "GuiContext.h"

inline cyanvne::runtime::GuiContext::GuiContext(const std::shared_ptr<WindowContext>& window, const std::shared_ptr<char>& font_data, uint64_t font_size, float size_pixels, float scale)
    : font_size_(font_size), font_pixels_size_(size_pixels)
{
    SDL_LogInfo(SDL_LOG_CATEGORY_APPLICATION, "Try to Creating GUI Context");
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    io = &ImGui::GetIO();
    io->ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;     // Enable Keyboard Controls
    io->ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;      // Enable Gamepad Controls

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
