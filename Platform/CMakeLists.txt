
set(CyanVNEPlatform_SRC
  "GuiContext/GuiContext.h"
  "GuiContext/GuiContext.cpp"
  "WMContext/WMContext.h"
  "WindowContext/WindowContext.h"
  "WindowContext/WindowContext.cpp"
  "GuiContext/GuiContext.h"
  "GuiContext/GuiContext.cpp"
  "PlatformException/PlatformException.h"
  "EventBus/EventBus.h"
  "EventBus/EventBus.cpp"
  "Monitor/Monitor.h"
  "Monitor/Monitor.cpp"
        ErrorEvent/ErrorEvent.h
        StreamUniversalImpl/StreamUniversalImpl.cpp
        "StreamUniversalImpl/StreamUniversalImpl.h"
        "UniversalPathToStream/UniversalPathToStream.h"
        "Algorithm/Binarization/Binarization.cpp"
        "Algorithm/Binarization/Binarization.h"
        "Algorithm/ContourTracing/ContourTracing.cpp"
        "Algorithm/ContourTracing/ContourTracing.h"
        Algorithm/Simplification/Simplification.cpp
        Algorithm/Simplification/Simplification.h
        Algorithm/Polypartition/polypartition.cpp
        Algorithm/Polypartition/polypartition.h
        Thread/UnifiedConcurrencyManager.h
        GuiContext/Detail/imgui_impl_bgfx.cpp
        GuiContext/Detail/imgui_impl_bgfx.h
        GuiContext/Detail/fs_ocornut_imgui.bin.h
        GuiContext/Detail/vs_ocornut_imgui.bin.h
        FontManager/FontManager.cpp
        FontManager/FontManager.h
)

add_library(CyanVNEPlatform STATIC ${CyanVNEPlatform_SRC})

set(CyanVNEPlatform_Require
  CyanVNECore
  SDL3-static
  SDL3_ttf::SDL3_ttf
  SDL3_image::SDL3_image
  imgui
  Boost::lockfree
  Boost::pool
        bgfx
        glm::glm
        polypartition
        Boost::asio
        Boost::fiber
)

target_link_libraries(CyanVNEPlatform PUBLIC ${CyanVNEPlatform_Require})

if (CMAKE_VERSION VERSION_GREATER 3.12)
  set_property(TARGET CyanVNEPlatform PROPERTY CXX_STANDARD 23)
endif()