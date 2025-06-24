cmake_minimum_required (VERSION 3.15.7)

if (POLICY CMP0141)
  cmake_policy(SET CMP0141 NEW)
  set(CMAKE_MSVC_DEBUG_INFORMATION_FORMAT "$<IF:$<AND:$<C_COMPILER_ID:MSVC>,$<CXX_COMPILER_ID:MSVC>>,$<$<CONFIG:Debug,RelWithDebInfo>:EditAndContinue>,$<$<CONFIG:Debug,RelWithDebInfo>:ProgramDatabase>>")
endif()

project ("CyanVNE")

if (CMAKE_HOST_WIN32)
  add_compile_definitions( IS_WIN32_SYS )
  add_compile_definitions ( _CRT_SECURE_NO_WARNINGS )
  add_compile_definitions(NOMINMAX)
endif()

# EnTT
add_subdirectory( "External/entt" )

# Boost
add_subdirectory( "External/boost" )

# Lua
set ( LUA_LANGUAGE "CXX" )
set ( LUA_LANGUAGE_SUPPORT "CXX" )
set ( LUA_USE_LONGJMP OFF )
set ( LUA_USE_APICHECK ON )
add_subdirectory( "External/lua-cmake" )

# SDL
set (BUILD_SHARED_LIBS false)
set(SDL_STATIC on)
add_subdirectory( "External/SDL" )
include_directories( "External/SDL/include" )

# SDL Extension
add_subdirectory( "External/SDL_ttf" )
include_directories( "External/SDL_ttf/include" )
add_subdirectory( "External/SDL_image" )
include_directories( "External/SDL_image/include" )

# Now the problem is that SDL_mixer and SDL_image cannot participate in cmake generation together when using cmake cache instead of rebuilding.
# Unless you don't use the vendor library
# This might be a bug

# add_subdirectory( External/SDL_mixer )
# include_directories( External/SDL_mixer/include )

# ImgUI
add_subdirectory( "External/pre_imgui" )
include_directories( "External/pre_imgui" )

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
add_subdirectory ("Resources")
add_subdirectory ("Runtime")
add_subdirectory ("Parser")