set(CMAKE_CXX_STANDARD 20)

set (IMGUISRC
  imgui/imgui.cpp
  imgui/imgui.h
  imgui/imconfig.h
  imgui/imgui_demo.cpp
  imgui/imgui_draw.cpp
  imgui/imgui_internal.h
  imgui/imgui_tables.cpp
  imgui/imgui_widgets.cpp
  imgui/imstb_rectpack.h
  imgui/imstb_textedit.h
  imgui/imstb_truetype.h
  imgui/backends/imgui_impl_sdl3.h
  imgui/backends/imgui_impl_sdl3.cpp
  imgui/backends/imgui_impl_opengl3.h
  imgui/backends/imgui_impl_opengl3.cpp
  imgui/backends/imgui_impl_sdlrenderer3.h
  imgui/backends/imgui_impl_sdlrenderer3.cpp
  imgui/misc/cpp/imgui_stdlib.cpp
)
add_library(imgui STATIC ${IMGUISRC})

target_include_directories(imgui PUBLIC imgui)
target_link_libraries(imgui PUBLIC SDL3-static)