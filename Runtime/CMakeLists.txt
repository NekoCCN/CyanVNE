
set(CyanVNERuntime_SRC
  "GameStateManager/GameStateManager.h"
  "GameStateManager/GameStateManager.cpp"
  "IGameState/IGameState.h"
  "GuiDebugState/GuiDebugState.h"
  "GuiDebugState/GuiDebugState.cpp"
  "RuntimeUtils/RuntimeUtils.h" "RuntimeUtils/RuntimeUtils.cpp" "MainMenuState/MainMenuState.h" "Components/Components.h" "Scene/Scene.h")

add_library(CyanVNERuntime STATIC ${CyanVNERuntime_SRC})

set(CyanVNERuntime_Require
  CyanVNECore
  CyanVNEPlatform
  lua_library_static
  EnTT::EnTT
)

target_link_libraries(CyanVNERuntime PUBLIC ${CyanVNERuntime_Require})

if (CMAKE_VERSION VERSION_GREATER 3.12)
  set_property(TARGET CyanVNERuntime PROPERTY CXX_STANDARD 23)
endif()