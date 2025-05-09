
set(CyanVNECore_SRC
  "Logger/Logger.h"
  "Logger/Logger.cpp"
  "CoreException/CoreException.h"
  "SpscQueue/SpscQueue.h"
  "Stream/Stream.h"
  "TSVector/TSVector.h"
  "Serialization/Serialization.h"
  "MemoryStreamImpl/MemoryStreamImpl.h"
  "MemoryStreamImpl/MemoryStreamImpl.cpp")

add_library(CyanVNECore STATIC ${CyanVNECore_SRC})

set(CyanVNECore_Require
  spdlog::spdlog
  SDL3-static
)

target_link_libraries(CyanVNECore PUBLIC ${CyanVNECore_Require})

if (CMAKE_VERSION VERSION_GREATER 3.12)
  set_property(TARGET CyanVNECore PROPERTY CXX_STANDARD 20)
endif()