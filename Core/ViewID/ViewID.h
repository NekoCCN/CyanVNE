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
}

#endif //CYANVNE_VIEWID_H
