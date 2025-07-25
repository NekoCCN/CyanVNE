//
// Created by Ext Culith on 2025/7/24.
//

#include "GraphicsContext.h"

#include "GraphicsContext.h"
#include "Core/Logger/Logger.h"
#include "Platform/PlatformException/PlatformException.h"
#include <bgfx/bgfx.h>
#include <SDL3/SDL.h>

using namespace cyanvne::platform;

namespace
{
    bgfx::RendererType::Enum mapRendererType(RendererType type)
    {
        switch (type)
        {
        case RendererType::OpenGL:
          return bgfx::RendererType::OpenGL;
        case RendererType::Vulkan:
          return bgfx::RendererType::Vulkan;
        case RendererType::D3D11:
          return bgfx::RendererType::Direct3D11;
        case RendererType::D3D12:
          return bgfx::RendererType::Direct3D12;
        case RendererType::Metal:
          return bgfx::RendererType::Metal;
        case RendererType::Auto:
        default:
          return bgfx::RendererType::Count;
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

GraphicsContext::GraphicsContext()
{
    if (SDL_Init(SDL_INIT_VIDEO) < 0)
    {
        cyanvne::core::GlobalLogger::getCoreLogger()->critical("Failed to initialize SDL: {}", SDL_GetError());
        throw cyanvne::exception::platformexception::CreateWindowContextException("Failed to initialize SDL");
    }
}

GraphicsContext::~GraphicsContext()
{
    shutdown();
    SDL_Quit();
}

void GraphicsContext::init(uint32_t width, uint32_t height, std::initializer_list<GfxResetOption> options, RendererType type)
{
    options_mask_ = mapResetOptions(options);

    bgfx::Init bgfx_init;
    bgfx_init.type = mapRendererType(type);
    bgfx_init.resolution.width = width;
    bgfx_init.resolution.height = height;
    bgfx_init.resolution.reset = options_mask_;

    if (!bgfx::init(bgfx_init))
    {
        cyanvne::core::GlobalLogger::getCoreLogger()->critical("Failed to initialize BGFX");
        throw cyanvne::exception::platformexception::CreateWindowContextException("Failed to initialize BGFX");
    }
}

void GraphicsContext::reset(uint32_t width, uint32_t height, std::initializer_list<GfxResetOption> options)
{
    options_mask_ = mapResetOptions(options);
    bgfx::reset(width, height, options_mask_);
}

void GraphicsContext::reset(uint32_t width, uint32_t height)
{
    bgfx::reset(width, height, options_mask_);
}

void GraphicsContext::shutdown()
{
    bgfx::shutdown();
}

void GraphicsContext::present()
{
    bgfx::frame();
}