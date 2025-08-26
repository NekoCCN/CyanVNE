
include_directories( "./" )

set(CyanVNEResources_SRC
  "ThemeMinimumRequire/ThemeMinimumRequire.h"
  "ResourcesPacker/ResourcesPacker.h"
  "ResourcesPacker/ResourcesPacker.cpp"
  "ResourcesException/ResourcesException.h"
  "ResourcesDefination/ResourcesDefination.h"
  "ThemeResourcesPacker/ThemeResourcesPacker.cpp"
  "ThemeResourcesPacker/ThemeResourcesPacker.h"
  "ResourcesManager/ResourcesManager.h"
  "ResourcesManager/ResourcesManager.cpp"
  "ThemeResourcesManager/ThemeResourcesManager.h"
  "Adapters/SdlIoStreamAdapter.h"
  "Adapters/SdlIoStreamAdapter.cpp"
  "UnifiedCacheManager/UnifiedCacheManager.h"
  "UnifiedCacheManager/UnifiedCacheManager.cpp"
 "ICachedResource/ICachedResource.h"
 "ResourceTypes/ResourceTypes.h"
 "ResourceTypes/ResourceType.cpp")

add_library(CyanVNEResources STATIC ${CyanVNEResources_SRC})

set(CyanVNEResources_Require
  CyanVNECore
  CyanVNEParser
  SDL3-static
        bgfx
)

target_link_libraries(CyanVNEResources PUBLIC ${CyanVNEResources_Require})

if (CMAKE_VERSION VERSION_GREATER 3.12)
  set_property(TARGET CyanVNEResources PROPERTY CXX_STANDARD 23)
endif()