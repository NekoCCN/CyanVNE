//
// Created by Ext Culith on 2025/7/3.
//

#pragma once
#ifndef DATASTRUCTURES_H
#define DATASTRUCTURES_H
#include <map>
#include <string>
#include <variant>

namespace cyanvne::ecs::commands
{
    class ICommand;
}

namespace cyanvne::parser::ecs
{
    class IComponentEmplacer;
}

namespace cyanvne::parser::data
{
    struct ResourceDeclaration
    {
        enum class Type
        {
            Script, Image, Audio
        };
        Type type;
        std::string path;
    };

    struct DataDeclaration
    {
        enum class Type
        {
            String, Number, Boolean
        };

        Type type = Type::Boolean;
        std::variant<std::string, double, bool> data;
    };

    struct EntityComponent
    {
        std::vector<ecs::IComponentEmplacer> components;
    };

    struct Declarations
    {
        std::map<std::string, ResourceDeclaration> resource_declarations;
        std::map<std::string, DataDeclaration> data_declarations;
        std::map<std::string, EntityComponent> entities;
    };

    struct DefinitionData
    {
        enum class Type
        {
            Boolean, Number, String
        };

        Type type = Type::Boolean;
        std::variant<bool, double, std::string> data;
    };

    struct SceneData
    {
        std::vector<EntityComponent> entities;
    };

    struct Definitions
    {
        std::map<std::string, DefinitionData> definitions;
        std::map<std::string, SceneData> scenes;
    };

    struct CommandsDefinition
    {
        std::map<std::string, std::unique_ptr<cyanvne::ecs::commands::ICommand>> commands_;
    };
}

#endif //DATASTRUCTURES_H
