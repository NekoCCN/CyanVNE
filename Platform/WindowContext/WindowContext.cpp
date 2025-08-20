#include "WindowContext.h"
#include "Core/Logger/Logger.h"
#include "Platform/PlatformException/PlatformException.h"
#include <bgfx/platform.h>
#include <bx/platform.h>

namespace cyanvne
{
    namespace platform
    {
        std::atomic<int> WindowContext::s_window_count_ = 0;
        uint32_t WindowContext::s_options_mask_ = BGFX_RESET_NONE;
    }
}

using namespace cyanvne::platform;

namespace
{
    bgfx::RendererType::Enum mapRendererType(RendererType type)
    {
        switch (type)
        {
            case RendererType::OpenGL:   return bgfx::RendererType::OpenGL;
            case RendererType::Vulkan:   return bgfx::RendererType::Vulkan;
            case RendererType::D3D11:    return bgfx::RendererType::Direct3D11;
            case RendererType::D3D12:    return bgfx::RendererType::Direct3D12;
            case RendererType::Metal:    return bgfx::RendererType::Metal;
            case RendererType::Auto:
            default:                     return bgfx::RendererType::Count;
        }
    }

    uint32_t mapResetOptions(std::initializer_list<GfxResetOption> options)
    {
        uint32_t mask = BGFX_RESET_NONE;
        for (const auto& option : options)
        {
            switch (option)
            {
                case GfxResetOption::Vsync:                 mask |= BGFX_RESET_VSYNC; break;
                case GfxResetOption::MsaaX2:                mask |= BGFX_RESET_MSAA_X2; break;
                case GfxResetOption::MsaaX4:                mask |= BGFX_RESET_MSAA_X4; break;
                case GfxResetOption::MsaaX8:                mask |= BGFX_RESET_MSAA_X8; break;
                case GfxResetOption::MsaaX16:               mask |= BGFX_RESET_MSAA_X16; break;
                case GfxResetOption::MaxAnisotropy:         mask |= BGFX_RESET_MAXANISOTROPY; break;
                case GfxResetOption::Capture:               mask |= BGFX_RESET_CAPTURE; break;
                case GfxResetOption::FlushAfterRender:      mask |= BGFX_RESET_FLUSH_AFTER_RENDER; break;
                case GfxResetOption::FlipAfterRender:       mask |= BGFX_RESET_FLIP_AFTER_RENDER; break;
                case GfxResetOption::SrgbBackbuffer:        mask |= BGFX_RESET_SRGB_BACKBUFFER; break;
                case GfxResetOption::Hdr10:                 mask |= BGFX_RESET_HDR10; break;
                case GfxResetOption::Hidpi:                 mask |= BGFX_RESET_HIDPI; break;
                case GfxResetOption::DepthClamp:            mask |= BGFX_RESET_DEPTH_CLAMP; break;
                case GfxResetOption::Suspend:               mask |= BGFX_RESET_SUSPEND; break;
                case GfxResetOption::TransparentBackbuffer: mask |= BGFX_RESET_TRANSPARENT_BACKBUFFER; break;
            }
        }
        return mask;
    }
}

WindowContext::WindowContext(
        const char* title, uint32_t width, uint32_t height,
        std::initializer_list<GfxResetOption> options,
        RendererType type, uint32_t target_fps)
        : window_{nullptr},
          width_{width},
          height_{height},
          target_frame_duration_{0},
          frame_buffer_handle_{BGFX_INVALID_HANDLE}
{
    if (target_fps > 0)
    {
        target_frame_duration_ = std::chrono::duration<double>(1.0 / target_fps);
    }

    window_ = SDL_CreateWindow(title, static_cast<int>(width), static_cast<int>(height), SDL_WINDOW_RESIZABLE | SDL_WINDOW_HIDDEN);
    if (!window_)
    {
        cyanvne::core::GlobalLogger::getCoreLogger()->critical("Failed to create window: {}", SDL_GetError());
        throw cyanvne::exception::platformexception::CreateWindowContextException("Failed to create window");
    }

    if (s_window_count_.fetch_add(1) == 0)
    {
        cyanvne::core::GlobalLogger::getCoreLogger()->info("First window created. Initializing BGFX using this window...");

        void* nwh = getNativeWindowHandle();
        if (!nwh)
        {
            s_window_count_--;
            SDL_DestroyWindow(window_);
            window_ = nullptr;
            throw cyanvne::exception::platformexception::CreateWindowContextException("Failed to get native handle for BGFX init");
        }

        bgfx::PlatformData pd{};
        pd.nwh = nwh;
        bgfx::setPlatformData(pd);

        bgfx::Init bgfx_init;
        bgfx_init.type = mapRendererType(type);
        bgfx_init.resolution.width = width;
        bgfx_init.resolution.height = height;
        s_options_mask_ = mapResetOptions(options);
        bgfx_init.resolution.reset = s_options_mask_;

        if (!bgfx::init(bgfx_init))
        {
            s_window_count_--;
            SDL_DestroyWindow(window_);
            window_ = nullptr;

            cyanvne::core::GlobalLogger::getCoreLogger()->critical("Failed to initialize BGFX");
            throw cyanvne::exception::platformexception::CreateWindowContextException("Failed to initialize BGFX");
        }
    }

    void* native_handle = getNativeWindowHandle();
    frame_buffer_handle_ = bgfx::createFrameBuffer(native_handle, width_, height_);
    if (!bgfx::isValid(frame_buffer_handle_))
    {
        if (s_window_count_.fetch_sub(1) == 1)
        {
            bgfx::shutdown();
        }
        SDL_DestroyWindow(window_);
        window_ = nullptr;

        cyanvne::core::GlobalLogger::getCoreLogger()->critical("Failed to create frame buffer for window: {}", title);
        throw cyanvne::exception::platformexception::CreateWindowContextException("Failed to create frame buffer");
    }

    renderer_type_ = bgfx::getCaps()->rendererType;

    showWindow();
}

WindowContext::~WindowContext()
{
    if (bgfx::isValid(frame_buffer_handle_))
    {
        bgfx::destroy(frame_buffer_handle_);
    }

    if (window_)
    {
        SDL_DestroyWindow(window_);
    }

    if (s_window_count_.fetch_sub(1) == 1)
    {
        cyanvne::core::GlobalLogger::getCoreLogger()->info("Last window destroyed. Shutting down BGFX.");
        bgfx::shutdown();
    }
}

void* WindowContext::getNativeWindowHandle()
{
    SDL_PropertiesID props = SDL_GetWindowProperties(window_);
#if BX_PLATFORM_LINUX
    return (void*)SDL_GetNumberProperty(props, SDL_PROP_WINDOW_X11_WINDOW_NUMBER, 0);
#elif BX_PLATFORM_WINDOWS
    return SDL_GetPointerProperty(props, SDL_PROP_WINDOW_WIN32_HWND_POINTER, nullptr);
#elif BX_PLATFORM_OSX
    return SDL_GetPointerProperty(props, SDL_PROP_WINDOW_COCOA_WINDOW_POINTER, nullptr);
#elif BX_PLATFORM_ANDROID
    return SDL_GetPointerProperty(props, SDL_PROP_WINDOW_ANDROID_WINDOW_POINTER, nullptr);
#elif BX_PLATFORM_EMSCRIPTEN
    const char* canvas_id = SDL_GetStringProperty(props, SDL_PROP_WINDOW_EMSCRIPTEN_CANVAS_ID_STRING, nullptr);
    if (canvas_id)
    {
        emscripten_canvas_selector_ = "#" + std::string(canvas_id);
        return (void*)emscripten_canvas_selector_.c_str();
    }
    return nullptr;
#else
    return nullptr;
#endif
}

void WindowContext::reset(uint32_t width, uint32_t height)
{
    width_ = width;
    height_ = height;

    bgfx::reset(width_, height_, s_options_mask_);

    if (bgfx::isValid(frame_buffer_handle_))
    {
        bgfx::destroy(frame_buffer_handle_);
    }

    void* native_handle = getNativeWindowHandle();
    if (native_handle == nullptr)
    {
        cyanvne::core::GlobalLogger::getCoreLogger()->error("Failed to get native handle on reset");
        return;
    }

    frame_buffer_handle_ = bgfx::createFrameBuffer(native_handle, width_, height_);
}

SDL_Window* WindowContext::getWindowHandle() const
{
    return window_;
}

bgfx::FrameBufferHandle WindowContext::getFrameBufferHandle() const
{
    return frame_buffer_handle_;
}

std::chrono::duration<double> WindowContext::getTargetFrameDuration() const
{
    return target_frame_duration_;
}

void WindowContext::getWindowSize(int32_t* width, int32_t* height) const
{
    SDL_GetWindowSize(window_, width, height);
}

void WindowContext::showWindow()
{
    SDL_ShowWindow(window_);
}

void WindowContext::hideWindow()
{
    SDL_HideWindow(window_);
}

void WindowContext::setWindowPosition(int32_t x, int32_t y)
{
    SDL_SetWindowPosition(window_, x, y);
}

void WindowContext::setWindowSize(int32_t width, int32_t height)
{
    SDL_SetWindowSize(window_, width, height);

    reset(static_cast<uint32_t>(width), static_cast<uint32_t>(height));
}

void WindowContext::setWindowFullscreen(bool fullscreen)
{
    SDL_SetWindowFullscreen(window_, fullscreen);
}

bool WindowContext::isMinimized() const
{
    return (SDL_GetWindowFlags(window_) & SDL_WINDOW_MINIMIZED);
}

bool WindowContext::isOccluded() const
{
    return (SDL_GetWindowFlags(window_) & SDL_WINDOW_OCCLUDED);
}