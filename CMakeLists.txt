cmake_minimum_required (VERSION 3.16)

project (CyanVNE VERSION 0.1.0)

if (CMAKE_HOST_WIN32)
  add_compile_definitions( IS_WIN32_SYS )
  add_compile_definitions ( _CRT_SECURE_NO_WARNINGS )
  add_compile_definitions( NOMINMAX )
endif()

if(MSVC)
  add_compile_options("/Zc:__cplusplus" "/Zc:preprocessor")
  add_compile_options("/EHsc")
endif()

# Boost
add_subdirectory( "External/boost" )
include_directories( "External/boost/fiber/examples" )

# Sol2
add_subdirectory( "External/sol2" )

# EnTT
add_subdirectory( "External/entt" )

# GLM
add_subdirectory( "External/glm" )

# SDL
set (BUILD_SHARED_LIBS false)
set(SDL_STATIC on)
add_subdirectory( "External/SDL" )
include_directories( "External/SDL/include" )

# SDL Extension
set(SDLTTF_VENDORED TRUE)
add_subdirectory( "External/SDL_ttf" )
include_directories( "External/SDL_ttf/include" )
set(SDLIMAGE_VENDORED TRUE)
set(SDLIMAGE_AVIF OFF CACHE BOOL "Disable internal AVIF to use top-level aom" FORCE)
add_subdirectory( "External/SDL_image" )
include_directories( "External/SDL_image/include" )

# Now the problem is that SDL_mixer and SDL_image cannot participate in cmake generation together when using cmake cache instead of rebuilding.
# Unless you don't use the vendor library
# This might be a bug

# add_subdirectory( External/SDL_mixer )
# include_directories( External/SDL_mixer/include )

# Bgfx
set(BGFX_SHADER_INCLUDE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/External/bgfx.cmake/bgfx/src")

add_subdirectory ( "External/bgfx.cmake" )
include_directories ( "External/bgfx.cmake/bgfx/include" )
include_directories ( "External/bgfx.cmake/bx/include" )
include_directories ( "External/bgfx.cmake/bimg/include" )

# ImgUI
add_subdirectory( "External/pre_imgui" )
include_directories( "External/pre_imgui" )

# PolyPartition
add_subdirectory( "External/pre_polypartition" )

# SoLoud
add_compile_definitions ( WITH_MINIAUDIO )
add_subdirectory ( "External/pre_soloud" )
include_directories ( "External/pre_soloud/soloud/include" )

# Logger
add_subdirectory( "External/spdlog" )
include_directories( "External/spdlog/include" )

# YAML-cpp
add_subdirectory( "External/yaml-cpp" )
include_directories( "External/yaml-cpp/include" )

# CyanVNE SRC
include_directories( "./" )
add_subdirectory ("CyanVNE")
add_subdirectory ("Core")
add_subdirectory ("Platform")
add_subdirectory ("Audio")
add_subdirectory ("Resources")
add_subdirectory ("Runtime")
add_subdirectory ("Parser")
add_subdirectory ("Shaders")