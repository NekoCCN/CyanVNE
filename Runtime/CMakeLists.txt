set(CyanVNERuntime_SRC
        "GameStateManager/GameStateManager.h"
        "GameStateManager/GameStateManager.cpp"
        "EcsGameState/EcsGameState.h"
        "RuntimeUtils/RuntimeUtils.h"
        "RuntimeUtils/RuntimeUtils.cpp"
        "Components/Components.h"
        "Systems/Systems.h"
        "Systems/Systems.cpp"
        "EcsGameState/EcsGameState.cpp"
        "RuntimeException/RuntimeException.h"
        Renderer/MeshBatchRenderer/MeshBatchRenderer.cpp
        Renderer/MeshBatchRenderer/MeshBatchRenderer.h
)

add_library(CyanVNERuntime STATIC ${CyanVNERuntime_SRC})

set(CyanVNERuntime_Require
        CyanVNECore
        CyanVNEPlatform
        CyanVNEAudio
        EnTT::EnTT
        glm::glm
        CyanVNEShaders
)

target_link_libraries(CyanVNERuntime PUBLIC ${CyanVNERuntime_Require})

if (CMAKE_VERSION VERSION_GREATER 3.12)
  set_property(TARGET CyanVNERuntime PROPERTY CXX_STANDARD 23)
endif()