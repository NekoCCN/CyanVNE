
set(CyanVNEPlatform_SRC
  "GuiContext/GuiContext.h"
  "GuiContext/GuiContext.cpp"
  "WMContext/WMContext.h"
  "WindowContext/WindowContext.h"
  "WindowContext/WindowContext.cpp"
  "PlatformException/PlatformException.h"
  "BasicRender/BasicRender.h"
  "BasicRender/BasicRender.cpp"
  "EventBus/EventBus.h"
  "EventBus/EventBus.cpp"
  "Monitor/Monitor.h"
  "Monitor/Monitor.cpp"
)

add_library(CyanVNEPlatform STATIC ${CyanVNEPlatform_SRC})

set(CyanVNEPlatform_Require
  CyanVNECore
  SDL3-static
  imgui
  Boost::lockfree
)

target_link_libraries(CyanVNEPlatform PUBLIC ${CyanVNEPlatform_Require})

if (CMAKE_VERSION VERSION_GREATER 3.12)
  set_property(TARGET CyanVNEPlatform PROPERTY CXX_STANDARD 20)
endif()