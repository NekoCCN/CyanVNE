cmake_minimum_required (VERSION 3.15.7)

project (CyanVNE VERSION 0.1.0)

if (CMAKE_HOST_WIN32)
  add_compile_definitions( IS_WIN32_SYS )
  add_compile_definitions ( _CRT_SECURE_NO_WARNINGS )
  add_compile_definitions( NOMINMAX )
endif()

# Boost
add_subdirectory( "External/boost" )

# LuaJIT
include(ExternalProject)

set(LUAJIT_SUBMODULE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/External/pre_LuaJIT/LuaJIT)
set(LUAJIT_INSTALL_DIR ${CMAKE_CURRENT_BINARY_DIR}/luajit_install)

if(ANDROID)
  if(ANDROID_ABI STREQUAL "arm64-v8a")
    set(LUAJIT_CROSS_PREFIX "aarch64-linux-android-")
    set(LUAJIT_TARGET_ARCH "arm64")
  endif()
  set(LUAJIT_BUILD_COMMAND ${CMAKE_MAKE_PROGRAM} -C ${LUAJIT_SUBMODULE_DIR} HOST_CC="gcc" CROSS=${LUAJIT_CROSS_PREFIX} TARGET_SYS=Linux TARGET_ARCH=${LUAJIT_TARGET_ARCH} amalg)
  set(LUAJIT_INSTALL_COMMAND ${CMAKE_MAKE_PROGRAM} -C ${LUAJIT_SUBMODULE_DIR} install DESTDIR=${LUAJIT_INSTALL_DIR} PREFIX=)
  set(LUAJIT_STATIC_LIB ${LUAJIT_INSTALL_DIR}/lib/libluajit-5.1.a)
  set(LUAJIT_INCLUDE_DIR ${LUAJIT_INSTALL_DIR}/include/luajit-2.1)
elseif(WIN32 AND NOT MINGW)
  message(STATUS "Configuring LuaJIT for Windows (MSVC/Clang) using msvcbuild.bat")

  set(LUAJIT_INSTALL_COMMAND "")
  set(LUAJIT_BUILD_COMMAND ${LUAJIT_SUBMODULE_DIR}/msvcbuild.bat static)
  set(LUAJIT_STATIC_LIB ${LUAJIT_SUBMODULE_DIR}/src/lua51.lib)
  set(LUAJIT_INCLUDE_DIR ${LUAJIT_SUBMODULE_DIR}/src)
else()
  message(STATUS "Configuring LuaJIT for Unix-like environment (Linux/macOS/MinGW)")

  set(LUAJIT_BUILD_COMMAND ${CMAKE_MAKE_PROGRAM} -C ${LUAJIT_SUBMODULE_DIR} amalg)
  set(LUAJIT_INSTALL_COMMAND ${CMAKE_MAKE_PROGRAM} -C ${LUAJIT_SUBMODULE_DIR} install DESTDIR=${LUAJIT_INSTALL_DIR} PREFIX=)
  set(LUAJIT_STATIC_LIB ${LUAJIT_INSTALL_DIR}/lib/libluajit-5.1.a)
  set(LUAJIT_INCLUDE_DIR ${LUAJIT_INSTALL_DIR}/include/luajit-2.1)
endif()


ExternalProject_Add(
        luajit_external
        GIT_REPOSITORY    ${LUAJIT_SUBMODULE_DIR}
        UPDATE_COMMAND    ""
        BUILD_COMMAND     ${LUAJIT_BUILD_COMMAND}
        INSTALL_COMMAND   ${LUAJIT_INSTALL_COMMAND}
        BUILD_BYPRODUCTS  ${LUAJIT_STATIC_LIB}
)

add_library(LuaJIT::LuaJIT STATIC IMPORTED GLOBAL)

set_target_properties(LuaJIT::LuaJIT PROPERTIES
        IMPORTED_LOCATION         "${LUAJIT_STATIC_LIB}"
        INTERFACE_INCLUDE_DIRECTORIES "${LUAJIT_INCLUDE_DIR}"
)

add_dependencies(LuaJIT::LuaJIT luajit_external)

if(UNIX)
  target_link_libraries(LuaJIT::LuaJIT INTERFACE dl m)
endif()

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