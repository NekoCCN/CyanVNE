
set(CyanVNERuntime_SRC
  "GameStateManager/GameStateManager.h"
  "GameStateManager/GameStateManager.cpp"
  "IGameState/IGameState.h"
  "GuiDebugState/GuiDebugState.h"
  "GuiDebugState/GuiDebugState.cpp"
  "RuntimeUtils/RuntimeUtils.h"
  "RuntimeUtils/RuntimeUtils.cpp"
  "MainMenuState/MainMenuState.h"
  "Components/Components.h"
  "Scene/Scene.h"
  "Systems/Systems.h"
  "Systems/Systems.cpp"
  "Scene/Scene.cpp"
 "GuiSystem/GuiSystem.h" "GuiSystem/GuiSystem.cpp" "IGuiWindow/IGuiWindow.h" "ICommand/ICommand.h" "IGameState/IGameState.cpp" "Commands/Commands.h" "Events/Events.h"
        "RuntimeException/RuntimeException.h"
        "Events/AudioEvents.h"
        "RendererService/RendererService.cpp"
        "RendererService/RendererService.h"
)

add_library(CyanVNERuntime STATIC ${CyanVNERuntime_SRC})

set(CyanVNERuntime_Require
  CyanVNECore
  CyanVNEPlatform
    CyanVNEAudio
  lua_library_static
  EnTT::EnTT
        glm::glm
        CyanVNEShaders
)

target_link_libraries(CyanVNERuntime PUBLIC ${CyanVNERuntime_Require})

if (CMAKE_VERSION VERSION_GREATER 3.12)
  set_property(TARGET CyanVNERuntime PROPERTY CXX_STANDARD 23)
endif()