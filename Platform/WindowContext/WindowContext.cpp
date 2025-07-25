#include "WindowContext.h"
#include "Core/Logger/Logger.h"
#include "Platform/PlatformException/PlatformException.h"
#include <bgfx/platform.h>

#if defined(__linux__) || defined(__unix__)
    #define BX_PLATFORM_LINUX 1
#elif defined(_WIN32) || defined(_WIN64)
    #define BX_PLATFORM_WINDOWS 1
#elif defined(__APPLE__)
    #include "TargetConditionals.h"
    #if TARGET_OS_MAC && !TARGET_OS_IPHONE
        #define BX_PLATFORM_OSX 1
    #endif
#elif defined(__ANDROID__)
    #define BX_PLATFORM_ANDROID 1
#elif defined(__EMSCRIPTEN__)
    #define BX_PLATFORM_EMSCRIPTEN 1
#endif

using namespace cyanvne::platform;

WindowContext::WindowContext(const char* title, uint32_t width, uint32_t height, uint32_t target_fps)
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

    void* native_handle = getNativeWindowHandle();
    if (native_handle == nullptr)
    {
        cyanvne::core::GlobalLogger::getCoreLogger()->critical("Failed to get native window handle for window: {}", title);
        throw cyanvne::exception::platformexception::CreateWindowContextException("Failed to get native window handle");
    }

    frame_buffer_handle_ = bgfx::createFrameBuffer(native_handle, width_, height_);
    if (!bgfx::isValid(frame_buffer_handle_))
    {
        cyanvne::core::GlobalLogger::getCoreLogger()->critical("Failed to create frame buffer for window: {}", title);
        throw cyanvne::exception::platformexception::CreateWindowContextException("Failed to create frame buffer");
    }

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
}

void* WindowContext::getNativeWindowHandle()
{
    SDL_PropertiesID props = SDL_GetWindowProperties(window_);
#if BX_PLATFORM_LINUX
    return (void*)SDL_GetNumberProperty(props, "SDL.window.x11.window", 0);
#elif BX_PLATFORM_WINDOWS
    return SDL_GetPointerProperty(props, "SDL.window.win32.window", nullptr);
#elif BX_PLATFORM_OSX
    return SDL_GetPointerProperty(props, "SDL.window.cocoa.window", nullptr);
#elif BX_PLATFORM_ANDROID
    return SDL_GetPointerProperty(props, "SDL.window.android.window", nullptr);
#elif BX_PLATFORM_EMSCRIPTEN
    return SDL_GetPointerProperty(props, "SDL.window.html5.canvas", nullptr);
#else
    return nullptr;
#endif
}

void WindowContext::reset(uint32_t width, uint32_t height)
{
    width_ = width;
    height_ = height;

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