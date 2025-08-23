cmake_minimum_required(VERSION 3.16)

set(ALL_GENERATED_SHADER_FILES "")

if(NOT EXISTS ${BGFX_SHADER_INCLUDE_PATH})
  message(FATAL_ERROR "BGFX include directory not found at '${BGFX_SHADER_INCLUDE_PATH}'. Please correct the path in Shaders/CMakeLists.txt.")
endif()

file(GLOB_RECURSE VARYING_FILES "*/varying.def.glsl")

foreach(VARYING_FILE ${VARYING_FILES})
  get_filename_component(SHADER_DIR ${VARYING_FILE} DIRECTORY)
  message(STATUS "Found shader directory: ${SHADER_DIR}")

  file(GLOB SHADER_SOURCES "${SHADER_DIR}/*.glsl")

  foreach(SHADER_FILE ${SHADER_SOURCES})
    get_filename_component(SHADER_NAME ${SHADER_FILE} NAME)
    if(SHADER_NAME STREQUAL "varying.def.glsl")
      continue()
    endif()

    set(OUTPUT_DIR "${SHADER_DIR}/bin")
    set(SHADER_TYPE "")
    if(SHADER_NAME MATCHES "^vs_")
      set(SHADER_TYPE "VERTEX")
    elseif(SHADER_NAME MATCHES "^fs_")
      set(SHADER_TYPE "FRAGMENT")
    elseif(SHADER_NAME MATCHES "^cs_")
      set(SHADER_TYPE "COMPUTE")
    endif()

    if(SHADER_TYPE)
      set(CURRENTLY_GENERATED_FILES "")
      bgfx_compile_shaders(
              TYPE ${SHADER_TYPE}
              SHADERS ${SHADER_FILE}
              VARYING_DEF ${VARYING_FILE}
              OUTPUT_DIR ${OUTPUT_DIR}
              AS_HEADERS
              OUT_FILES_VAR CURRENTLY_GENERATED_FILES
      )
      list(APPEND ALL_GENERATED_SHADER_FILES ${CURRENTLY_GENERATED_FILES})
    endif()
  endforeach()
endforeach()

add_custom_target(
        compile_shaders ALL
        DEPENDS ${ALL_GENERATED_SHADER_FILES}
)

add_library(CyanVNEShaders INTERFACE)

add_dependencies(CyanVNEShaders compile_shaders)

target_include_directories(CyanVNEShaders INTERFACE
        ${CMAKE_CURRENT_SOURCE_DIR}
)