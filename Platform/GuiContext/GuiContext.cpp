#include "GuiContext.h"
#include "Core/Logger/Logger.h"
#include "Core/ViewID/ViewID.h"
#include <bx/platform.h>
#include <Platform/PlatformException/PlatformException.h>

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

    ImGui_Implbgfx_Init((int16_t)core::RenderLayer::UI);
    bgfx::RendererType::Enum renderer_type = window->getRendererType();
    switch (renderer_type)
    {
        case bgfx::RendererType::OpenGL:
            ImGui_ImplSDL3_InitForOpenGL(window->getWindowHandle(), nullptr);
            break;
        case bgfx::RendererType::Vulkan:
            ImGui_ImplSDL3_InitForVulkan(window->getWindowHandle());
            break;
        case bgfx::RendererType::Direct3D11:
            ImGui_ImplSDL3_InitForD3D(window->getWindowHandle());
            break;
        case bgfx::RendererType::Direct3D12:
            ImGui_ImplSDL3_InitForD3D(window->getWindowHandle());
            break;
        case bgfx::RendererType::Metal:
            ImGui_ImplSDL3_InitForMetal(window->getWindowHandle());
            break;
        default:
            core::GlobalLogger::getCoreLogger()->error("Unsupported renderer type for ImGui initialization: {}", bgfx::getRendererName(renderer_type));
            throw cyanvne::exception::platformexception::CreateWindowContextException("Unsupported renderer type for ImGui initialization");
    }


    int32_t h, w;
    window->getWindowSize(&w, &h);

    if (font_size_ > 0)
    {
        font_data_ = new char[font_size_];
        memcpy(font_data_, font_data.data(), font_size_);
        
        core::GlobalLogger::getCoreLogger()->info("Built-in Font Loaded, memory size: {:d}, pixel: {:.2f}",
            font_size_, size_pixels);

        io->Fonts->AddFontFromMemoryTTF(font_data_, static_cast<int>(font_size_), size_pixels);
        ImGui::GetIO().FontGlobalScale = calculateScreenScale(w, h, size_pixels);
    }

    ImGui::GetIO().FontGlobalScale = calculateScreenScale(w, h, font_pixels_size_);

    setupImGuiStyle(false, 0.7f);
}
    