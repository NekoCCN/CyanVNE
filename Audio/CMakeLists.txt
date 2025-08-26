
set(CyanVNEAudio_SRC
        "IAudioEngine/IAudioEngine.h"
        SoloudAudioEngine/SoloudAudioEngine.cpp
        SoloudAudioEngine/SoloudAudioEngine.h
        AudioManager/AudioManager.h
        Events/AudioEvents.h
)

add_library(CyanVNEAudio STATIC ${CyanVNEAudio_SRC})

set(CyanVNEAudio_Require
        CyanVNECore
        CyanVNEPlatform
        soloud
        EnTT::EnTT
        glm::glm
)

target_link_libraries(CyanVNEAudio PUBLIC ${CyanVNEAudio_Require})

if (CMAKE_VERSION VERSION_GREATER 3.12)
  set_property(TARGET CyanVNEAudio PROPERTY CXX_STANDARD 23)
endif()