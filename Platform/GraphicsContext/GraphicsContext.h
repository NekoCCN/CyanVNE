//
// Created by Ext Culith on 2025/7/24.
//

#ifndef GRAPHICSCONTEXT_H
#define GRAPHICSCONTEXT_H

#include <cstdint>
#include <initializer_list>

namespace cyanvne
{
    namespace platform
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

        class GraphicsContext
        {
        public:
            GraphicsContext();
            ~GraphicsContext();

            GraphicsContext(const GraphicsContext&) = delete;
            GraphicsContext& operator=(const GraphicsContext&) = delete;
            GraphicsContext(GraphicsContext&&) = delete;
            GraphicsContext& operator=(GraphicsContext&&) = delete;

            void init(
                uint32_t width,
                uint32_t height,
                std::initializer_list<GfxResetOption> options,
                RendererType type = RendererType::Auto
            );

            void reset(uint32_t width, uint32_t height, std::initializer_list<GfxResetOption> options);
            void reset(uint32_t width, uint32_t height);
            void shutdown();
            void present();

        private:
            uint32_t options_mask_;
        };
    }
}


#endif //GRAPHICSCONTEXT_H
