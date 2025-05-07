set(CMAKE_CXX_STANDARD 20)

file(GLOB SOLOUDSRC
  "soloud/src/audiosource/wav/*"
  "soloud/src/backend/miniaudio/*"
  "soloud/src/core/*"
  "soloud/src/c_api/*"
  "soloud/src/filter/*"
)


add_library(soloud STATIC ${SOLOUDSRC})

target_include_directories(soloud PUBLIC soloud/include )