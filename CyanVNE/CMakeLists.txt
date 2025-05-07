
add_executable (CyanVNE "CyanVNE.cpp" "CyanVNE.h")

if (CMAKE_VERSION VERSION_GREATER 3.12)
  set_property(TARGET CyanVNE PROPERTY CXX_STANDARD 20)
endif()