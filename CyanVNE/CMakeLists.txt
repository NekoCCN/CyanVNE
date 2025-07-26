
add_executable (CyanVNE "CyanVNE.cpp" "CyanVNE.h" "Application/Application.h" "ToolApplication/ToolApplication.h")

target_link_libraries (CyanVNE 
  CyanVNECore
  CyanVNEPlatform
  CyanVNEParser
  CyanVNEResources
  CyanVNERuntime
  EnTT::EnTT
  )

if (CMAKE_VERSION VERSION_GREATER 3.12)
  set_property(TARGET CyanVNE PROPERTY CXX_STANDARD 23)
endif()