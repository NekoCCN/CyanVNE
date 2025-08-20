#pragma once

#include <SDL3/SDL.h>
#include <cstdint>
#include <chrono>
#include <bgfx/bgfx.h>

namespace cyanvne::platform
{
    enum class RendererType
    {
        Auto,
        OpenGL,
        Vulkan,
        D3D11,
        D3D12,
        Metal
    };

    enum class GfxResetOption
    {
        Vsync,
        MsaaX2,
        MsaaX4,
        MsaaX8,
        MsaaX16,
        MaxAnisotropy,
        Capture,
        FlushAfterRender,
        FlipAfterRender,
        SrgbBackbuffer,
        Hdr10,
        Hidpi,
        DepthClamp,
        Suspend,
        TransparentBackbuffer
    };

    class WindowContext
    {
    public:
        WindowContext(
                const char *title,
                uint32_t width = 1280,
                uint32_t height = 720,
                std::initializer_list<GfxResetOption> options = {},
                RendererType type = RendererType::Auto,
                uint32_t target_fps = 0
        );

        ~WindowContext();

        WindowContext(const WindowContext &) = delete;

        WindowContext &operator=(const WindowContext &) = delete;

        WindowContext(WindowContext &&) = delete;

        WindowContext &operator=(WindowContext &&) = delete;

        void reset(uint32_t width, uint32_t height);

        [[nodiscard]] SDL_Window *getWindowHandle() const;

        [[nodiscard]] bgfx::FrameBufferHandle getFrameBufferHandle() const;

        [[nodiscard]] std::chrono::duration<double> getTargetFrameDuration() const;

        void getWindowSize(int32_t *width, int32_t *height) const;

        void showWindow();

        void hideWindow();

        void setWindowPosition(int32_t x, int32_t y);

        void setWindowSize(int32_t width, int32_t height);

        void setWindowFullscreen(bool fullscreen);

        [[nodiscard]] bool isMinimized() const;

        [[nodiscard]] bool isOccluded() const;

        [[nodiscard]] bgfx::RendererType::Enum getRendererType() const { return renderer_type_; }

        [[nodiscard]] SDL_Window* getNativeWindowHandle() const { return window_; }
    private:
        void *getNativeWindowHandle();

        SDL_Window *window_;
        uint32_t width_;
        uint32_t height_;
        std::chrono::duration<double> target_frame_duration_;
        bgfx::FrameBufferHandle frame_buffer_handle_;

        bgfx::RendererType::Enum renderer_type_;

        static std::atomic<int> s_window_count_;
        static uint32_t s_options_mask_;

#if BX_PLATFORM_EMSCRIPTEN
        std::string emscripten_canvas_selector_;
#endif
    };
}