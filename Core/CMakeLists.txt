
set(CyanVNECore_SRC
  "Logger/Logger.h"
  "Logger/Logger.cpp"
)

add_library(CyanVNECore STATIC ${CyanVNECore_SRC})


set(CyanVNECore_Require
  SDL3-static
  imgui
)

target_link_libraries(CyanVNECore PUBLIC ${Core_Require})

if (CMAKE_VERSION VERSION_GREATER 3.12)
  set_property(TARGET CyanVNECore PROPERTY CXX_STANDARD 17)
endif()