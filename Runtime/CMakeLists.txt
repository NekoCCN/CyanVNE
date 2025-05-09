
set(CyanVNERuntime_SRC
  "GuiContext/GuiContext.h"
  "GuiContext/GuiContext.cpp"
  "WMContext/WMContext.h"
  "WMContext/WMContext.cpp"
)

add_library(CyanVNERuntime STATIC ${CyanVNERuntime_SRC})

set(CyanVNERuntime_Require
  CyanVNECore
  SDL3-static
  SDL3_image::SDL3_image
  SDL3_ttf::SDL3_ttf
  imgui
  soloud
)

target_link_libraries(CyanVNERuntime PUBLIC ${CyanVNERuntime_Require})

if (CMAKE_VERSION VERSION_GREATER 3.12)
  set_property(TARGET CyanVNERuntime PROPERTY CXX_STANDARD 20)
endif()