cmake_minimum_required(VERSION 3.15)
project(LuaJIT LANGUAGES C CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_POSITION_INDEPENDENT_CODE ON)

file(GLOB LUAJIT_SRC
        "LuaJIT/src/*.c"
        "LuaJIT/src/*.h"
)

add_library(LuaJIT STATIC ${LUAJIT_SRC})

target_include_directories(LuaJIT PUBLIC "LuaJIT/src")

if(MSVC)
    target_compile_definitions(LuaJIT PRIVATE _CRT_SECURE_NO_WARNINGS)
endif()