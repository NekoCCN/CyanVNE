#include "GuiContext.h"

cyanvne::platform::GuiContext::GuiContext(const std::shared_ptr<WindowContext>& window,
    const std::vector<uint8_t>& font_data,
    float size_pixels,
    const std::set<std::string>& extra_languages_support)
    : font_size_(font_data.size()), font_pixels_size_(size_pixels)
{
    core::GlobalLogger::getCoreLogger()->info("Try to Creating GUI Context");
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    io = &ImGui::GetIO();
    io->ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;     // Enable Keyboard Controls
    io->ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;      // Enable Gamepad Controls

    core::GlobalLogger::getCoreLogger()->info("GUI Theme : Light");
    ImGui::StyleColorsLight();

    ImGui_ImplSDL3_InitForSDLRenderer(window->getWindowHinding(), window->getRendererHinding());
    ImGui_ImplSDLRenderer3_Init(window->getRendererHinding());


    if (font_size_ > 0)
    {
        font_data_ = new char[font_size_];
        memcpy(font_data_, font_data.data(), font_size_);
        
        core::GlobalLogger::getCoreLogger()->info("Built-in Font Loaded, memory size: {:d}, pixel: {:.2f}",
            font_size_, size_pixels);

        io->Fonts->AddFontFromMemoryTTF(font_data_, static_cast<int>(font_size_), size_pixels);
        ImGui::GetIO().FontGlobalScale = calculateScreenScale(static_cast<int>(window->getWindowRect().w),
            static_cast<int>(window->getWindowRect().h), size_pixels);
    }

    ImGui::GetIO().FontGlobalScale = calculateScreenScale(static_cast<float>(window->getWindowRect().w),
        static_cast<float>(window->getWindowRect().h), font_pixels_size_);

    setupImGuiStyle(false, 0.7f);
}
    