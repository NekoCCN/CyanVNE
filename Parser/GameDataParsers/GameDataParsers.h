//
// Created by Ext Culith on 2025/7/3.
//

#ifndef GAMEDATAPARSERS_H
#define GAMEDATAPARSERS_H

#pragma once
#include <Parser/Framework/Framework.h>
#include <Parser/Ecs/ComponentParsers.h>

#include "Parser/DataStructures/GameData.h"

namespace cyanvne::parser
{
    class EntityParser : public INodeParser
    {
    private:
        std::shared_ptr<NodeParserRegistry> component_parser_registry_;
    public:
        explicit EntityParser(std::shared_ptr<NodeParserRegistry> registry)
            : component_parser_registry_(std::move(registry))
        {  }

        std::unique_ptr<ParsedNodeData> parse(const YAML::Node& node, const NodeParserRegistry&) const override
        {
            auto components_map = util::getMapSequenceAsNodeMapElseThrow(node, "components", getParsableNodeType());

            data::EntityComponent entity_data;

            for (const auto& [name, comp_node] : components_map)
            {
                entity_data.components.push_back(
                    *(component_parser_registry_->parseNodeByKeyOrType(name, comp_node)->getAs<ecs::IComponentEmplacer>())
                );
            }
            return std::make_unique<ParsedNodeData>(entity_data, getParsableNodeType());
        }

        std::string getParsableNodeType() const override
        {
            return "entity";
        }
    };

    class ResourceDeclarationParser : public INodeParser
    {
    public:
        std::unique_ptr<ParsedNodeData> parse(const YAML::Node&, const NodeParserRegistry&) const override;
        std::string getParsableNodeType() const override;
    };

    class DataDeclarationParser : public INodeParser
    {
    public:
        std::unique_ptr<ParsedNodeData> parse(const YAML::Node&, const NodeParserRegistry&) const override;
        std::string getParsableNodeType() const override;
    };

    class DeclarationsParser : public INodeParser
    {
    public:
        std::unique_ptr<ParsedNodeData> parse(const YAML::Node&, const NodeParserRegistry&) const override;
        std::string getParsableNodeType() const override;
    };

    class VariableParser : public INodeParser
    {
    public:
        std::unique_ptr<ParsedNodeData> parse(const YAML::Node&, const NodeParserRegistry&) const override;
        std::string getParsableNodeType() const override;
    };

    class SceneParser : public INodeParser
    {
    public:
        std::unique_ptr<ParsedNodeData> parse(const YAML::Node&, const NodeParserRegistry&) const override;
        std::string getParsableNodeType() const override;
    };

    class DefinitionsParser : public INodeParser
    {
    public:
        std::unique_ptr<ParsedNodeData> parse(const YAML::Node&, const NodeParserRegistry&) const override;
        std::string getParsableNodeType() const override;
    };

    struct ParsedEntityData
    {
        std::vector<std::unique_ptr<ecs::IComponentEmplacer>> component_emplacers;
    };
}

#endif //GAMEDATAPARSERS_H
