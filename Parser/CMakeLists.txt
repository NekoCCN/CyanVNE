
set(CyanVNEParser_SRC
  "AppSettings/AppSettings.h"
  "AppSettings/AppSettingsParsers/AppSettingsParsers.cpp"
  "AppSettings/AppSettingsParsers/AppSettingsParsers.h"
  "ParserYamlInterface/ParserYamlInterface.h"
  "ParserException/ParserException.h"
  "ParserUtil/ParserUtil.h"
  "ParserUtil/ParserUtil.cpp"
  "ThemeConfig/ThemeConfig.h"
  "ThemeConfig/ThemeConfig.cpp"
  "ThemeConfigParser/ThemeConfigParser.h"
  "ThemeConfigParser/ThemeConfigParser.cpp"
  "Framework/Framework.h"
  "Framework/Framework.cpp"
  "ParserFactories/ParserFactories.h"
        "DataStructures/GameData.h"
        GameDataParsers/GameDataParsers.cpp
        GameDataParsers/GameDataParsers.h
        Ecs/ComponentParsers.cpp
        Ecs/ComponentParsers.h
        CommandsParser/CommandsParser.cpp
        CommandsParser/CommandsParser.h
        CommandParser/CommandParser.cpp
        CommandParser/CommandParser.h
)

add_library(CyanVNEParser STATIC ${CyanVNEParser_SRC})

set(CyanVNEParser_Require
  CyanVNECore
        CyanVNERuntime
  yaml-cpp
        EnTT::EnTT
)

target_link_libraries(CyanVNEParser PUBLIC ${CyanVNEParser_Require})

if (CMAKE_VERSION VERSION_GREATER 3.12)
  set_property(TARGET CyanVNEParser PROPERTY CXX_STANDARD 23)
endif()