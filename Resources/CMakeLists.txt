
include_directories( "./" )

set(CyanVNEResources_SRC
  "StreamUniversalImpl/StreamUniversalImpl.h"
  "StreamUniversalImpl/StreamUniversalImpl.cpp")

add_library(CyanVNEResources STATIC ${CyanVNEResources_SRC})

set(CyanVNEResources_Require
  CyanVNECore
  SDL3-static
)

target_link_libraries(CyanVNEResources PUBLIC ${CyanVNEResources_Require})

if (CMAKE_VERSION VERSION_GREATER 3.12)
  set_property(TARGET CyanVNEResources PROPERTY CXX_STANDARD 20)
endif()