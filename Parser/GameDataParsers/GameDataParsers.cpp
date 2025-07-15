//
// Created by Ext Culith on 2025/7/3.
//

#include "GameDataParsers.h"
#include "Parser/DataStructures/GameData.h"
#include "Parser/ParserUtil/ParserUtil.h"

namespace cyanvne::parser
{
    std::string ResourceDeclarationParser::getParsableNodeType() const
    {
        return "resource_declaration";
    }

    std::unique_ptr<ParsedNodeData> ResourceDeclarationParser::parse(const YAML::Node& node, const NodeParserRegistry&) const
    {
        data::ResourceDeclaration decl;
        decl.path = util::getScalarNodeElseThrow<std::string>(node, "path", getParsableNodeType());

        std::string type_str = util::getScalarNodeElseThrow<std::string>(node, "type", getParsableNodeType());
        if (type_str == "script")
        {
            decl.type = data::ResourceDeclaration::Type::Script;
        }
        else if (type_str == "image")
        {
            decl.type = data::ResourceDeclaration::Type::Image;
        }
        else if (type_str == "audio")
        {
            decl.type = data::ResourceDeclaration::Type::Audio;
        }
        else
        {
            throw exception::parserexception::ParserException("Resource Declaration Error", "Invalid type: " + type_str);
        }

        return std::make_unique<ParsedNodeData>(std::move(decl), getParsableNodeType());
    }

    std::string DataDeclarationParser::getParsableNodeType() const
    {
        return "data_declaration";
    }

    std::unique_ptr<ParsedNodeData> DataDeclarationParser::parse(const YAML::Node& node, const NodeParserRegistry&) const
    {
        data::DataDeclaration decl;
        const auto& data_node = util::getYamlNodeElseThrow(node, "data", getParsableNodeType());

        std::string type_str = util::getScalarNodeElseThrow<std::string>(node, "type", getParsableNodeType());

        try
        {
            if (type_str == "string")
            {
                decl.type = data::DataDeclaration::Type::String;
                decl.data = data_node.as<std::string>();
            }
            else if (type_str == "number")
            {
                decl.type = data::DataDeclaration::Type::Number;
                decl.data = data_node.as<double>();
            }
            else if (type_str == "boolean")
            {
                decl.type = data::DataDeclaration::Type::Boolean;
                decl.data = data_node.as<bool>();
            }
            else
            {
                throw exception::parserexception::ParserException("Data Declaration Error", "Invalid type: " + type_str);
            }
        }
        catch (const YAML::BadConversion& e)
        {
            throw exception::parserexception::ParserException(
                "Data Declaration Type Mismatch",
                "The value of 'data' does not match the declared type '" + type_str + "'. YAML error: " + e.what()
            );
        }

        return std::make_unique<ParsedNodeData>(std::move(decl), getParsableNodeType());
    }

    std::string DeclarationsParser::getParsableNodeType() const
    {
        return "declarations";
    }

    std::unique_ptr<ParsedNodeData> DeclarationsParser::parse(const YAML::Node& node, const NodeParserRegistry& registry) const
    {
        if (!node.IsMap())
        {
            throw exception::parserexception::ParserException("Declarations Parser Error", "Top-level 'declarations' node must be a map.");
        }

        data::Declarations all_declarations;

        for (const auto& pair : node)
        {
            std::string key = pair.first.as<std::string>();
            const YAML::Node& decl_node = pair.second;

            if (decl_node["path"] && decl_node["data"] && decl_node["components"])
            {
                throw exception::parserexception::ParserException(
                    "Declarations Parser Error",
                    "Declaration '" + key + "' cannot contain both 'path' 'data' and 'components' keys."
                );
            }

            if (decl_node["path"])
            {
                auto parsed_data = registry.parseNodeByKeyOrType("resource_declaration", decl_node);
                if (const auto* res_decl = parsed_data->getAs<data::ResourceDeclaration>())
                {
                    if (all_declarations.resource_declarations.contains(key))
                    {
                        throw exception::parserexception::ParserException(
                            "Resource Declaration Error",
                            "Resource declaration with key '" + key + "' already exists."
                        );
                    }
                    all_declarations.resource_declarations[key] = *res_decl;
                }
            }
            else if (decl_node["data"])
            {
                auto parsed_data = registry.parseNodeByKeyOrType("data_declaration", decl_node);
                 if (const auto* data_decl = parsed_data->getAs<data::DataDeclaration>())
                 {
                     if (all_declarations.data_declarations.contains(key))
                     {
                         throw exception::parserexception::ParserException(
                             "Data Declaration Error",
                             "Data declaration with key '" + key + "' already exists."
                         );
                     }
                    all_declarations.data_declarations[key] = *data_decl;
                }
            }
            else if (decl_node["components"])
            {
                auto parsed_data = registry.parseNodeByKeyOrType("entity", decl_node);
                if (const auto* entity_decl = parsed_data->getAs<data::EntityComponent>())
                {
                    if (all_declarations.entities.contains(key))
                    {
                        throw exception::parserexception::ParserException(
                            "Entity Declaration Error",
                            "Entity declaration with key '" + key + "' already exists."
                        );
                    }
                    all_declarations.entities[key] = *entity_decl;
                }
            }
            else
            {
                throw exception::parserexception::ParserException("Declarations Parser Error", "Declaration '" + key + "' must contain either a 'path' or a 'data' key.");
            }
        }
        return std::make_unique<ParsedNodeData>(std::move(all_declarations), getParsableNodeType());
    }

    std::unique_ptr<ParsedNodeData> SceneParser::parse(const YAML::Node& node, const NodeParserRegistry& registry) const
    {
        data::SceneData scene_data;

        if (!node.IsSequence())
        {
            throw exception::parserexception::ParserException("Scene Parser Error", "Top-level 'scene' node must be a sequence.");
        }

        for (const auto& entity_node : node)
        {
            if (!entity_node.IsMap())
            {
                throw exception::parserexception::ParserException("Scene Parser Error",
                    "Each entity in the scene must be a map.");
            }

            auto parsed_data = registry.parseNodeByKeyOrType("entity", entity_node);

            scene_data.entities.push_back(*parsed_data->getAs<data::EntityComponent>());
        }

        return std::make_unique<ParsedNodeData>(std::move(scene_data), getParsableNodeType());
    }

    std::string SceneParser::getParsableNodeType() const
    {
        return "scene";
    }

    std::unique_ptr<ParsedNodeData> VariableParser::parse(const YAML::Node& node, const NodeParserRegistry&) const
    {
        data::DefinitionData def;

        const auto& data_node = util::getYamlNodeElseThrow(node, "data", getParsableNodeType());

        std::string type_str = util::getScalarNodeElseThrow<std::string>(node, "type", getParsableNodeType());

        try
        {
            if (type_str == "boolean")
            {
                def.type = data::DefinitionData::Type::Boolean;
                def.data = data_node.as<bool>();
            }
            else if (type_str == "number")
            {
                def.type = data::DefinitionData::Type::Number;
                def.data = data_node.as<double>();
            }
            else if (type_str == "string")
            {
                def.type = data::DefinitionData::Type::String;
                def.data = data_node.as<std::string>();
            }
            else
            {
                throw exception::parserexception::ParserException("Definition Error", "Invalid type: " + type_str);
            }
        }
        catch (const YAML::BadConversion& e)
        {
            throw exception::parserexception::ParserException(
                "Definition Type Mismatch",
                "The value of 'data' does not match the declared type '" + type_str + "'. YAML error: " + e.what()
            );
        }

        return std::make_unique<ParsedNodeData>(std::move(def), getParsableNodeType());
    }

    std::string VariableParser::getParsableNodeType() const
    {
        return "variable";
    }

    std::unique_ptr<ParsedNodeData> DefinitionsParser::parse(const YAML::Node& node, const NodeParserRegistry& registry) const
    {
        data::Definitions all_definitions;
        auto map_data = util::getMapSequenceAsNodeMapElseThrow(node, "definition", "Definitions Block");

        for(const auto& [key, def_node] : map_data)
        {
            if (def_node.IsMap())
            {
                auto parsed_data = registry.parseNodeByKeyOrType("definition", def_node);
                if (const auto* def = parsed_data->getAs<data::DefinitionData>())
                {
                    all_definitions.definitions[key] = *def;
                }
            }
            else if (def_node.IsSequence())
            {
                auto parsed_data = registry.parseNodeByKeyOrType("scene", def_node);
                if (const auto* def = parsed_data->getAs<data::SceneData>())
                {
                    all_definitions.scenes[key] = *def;
                }
            }
            else
            {
                throw exception::parserexception::ParserException("Definitions Parser Error",
                    "Invalid definition node for key: " + key);
            }
        }
        return std::make_unique<ParsedNodeData>(std::move(all_definitions), getParsableNodeType());
    }

    std::string DefinitionsParser::getParsableNodeType() const
    {
        return "definitions";
    }
}