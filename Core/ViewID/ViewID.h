//
// Created by Ext Culith on 2025/8/14.
//

#ifndef CYANVNE_VIEWID_H
#define CYANVNE_VIEWID_H

#include <cstdint>

namespace cyanvne::core
{
    enum class RenderLayer : uint16_t
    {
        Skybox = 0,
        World3D = 10,
        Particles = 20,
        DebugLines = 100,
        UI = 200,
        LoadingScreen = 255
    };

    enum class RenderLayerMask : uint32_t
    {
        None         = 0,
        Default      = 1 << 0,
        Player       = 1 << 1,
        Enemy        = 1 << 2,
        Background   = 1 << 3,
        UI           = 1 << 30,
        Everything   = 0xFFFFFFFF
    };

    inline RenderLayerMask operator|(RenderLayerMask a, RenderLayerMask b)
    {
        return static_cast<RenderLayerMask>(static_cast<uint32_t>(a) | static_cast<uint32_t>(b));
    }

    inline RenderLayerMask operator&(RenderLayerMask a, RenderLayerMask b)
    {
        return static_cast<RenderLayerMask>(static_cast<uint32_t>(a) & static_cast<uint32_t>(b));
    }
}

#endif //CYANVNE_VIEWID_H
