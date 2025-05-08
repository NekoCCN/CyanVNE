
include_directories( "./" )

set(CyanVNEResource_SRC
  "StreamUniversialImpl/StreamUniversialImpl.h"
)

add_library(CyanVNEResource STATIC ${CyanVNEResource_SRC})

set(CyanVNEResource_Require
  CyanVNECore
  SDL3-static
)

target_link_libraries(CyanVNEResource PUBLIC ${CyanVNEResource_Require})

if (CMAKE_VERSION VERSION_GREATER 3.12)
  set_property(TARGET CyanVNEResource PROPERTY CXX_STANDARD 20)
endif()