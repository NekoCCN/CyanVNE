//
// Created by Ext Culith on 2025/7/3.
//

#pragma once
#ifndef DATASTRUCTURES_H
#define DATASTRUCTURES_H
#include <map>
#include <string>
#include <variant>

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
        Type type;
        std::variant<std::string, double, bool> data;
    };

    using DeclarationVariant = std::variant<ResourceDeclaration, DataDeclaration>;

    struct Declarations
    {
        std::map<std::string, DeclarationVariant> declarations;
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

    struct Definitions
    {
        std::map<std::string, DefinitionData> definitions;
    };

}

#endif //DATASTRUCTURES_H
