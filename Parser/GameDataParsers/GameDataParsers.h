//
// Created by Ext Culith on 2025/7/3.
//

#ifndef GAMEDATAPARSERS_H
#define GAMEDATAPARSERS_H

#pragma once
#include <Parser/Framework/Framework.h>
#include <Parser/Ecs/ComponentParsers.h>

namespace cyanvne::parser
{
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

    class DefinitionParser : public INodeParser
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

    class EntityParser : public INodeParser
    {
    private:
        std::shared_ptr<ecs::ComponentParserRegistry> component_parser_registry_;
    public:
        EntityParser(std::shared_ptr<ecs::ComponentParserRegistry> registry)
            : component_parser_registry_(std::move(registry))
        {  }

        std::unique_ptr<ParsedNodeData> parse(const YAML::Node& node, const NodeParserRegistry&) const override
        {
            auto components_map = util::getMapAsNodeMapElseThrow(node, "components", getParsableNodeType());

            ParsedEntityData entity_data;
            for (const auto& [name, comp_node] : components_map)
            {
                entity_data.component_emplacers.push_back(
                    component_parser_registry_->parseComponent(name, comp_node)
                );
            }
            return std::make_unique<ParsedNodeData>(entity_data, getParsableNodeType());
        }

        std::string getParsableNodeType() const override
        {
            return "entity";
        }
    };
}

#endif //GAMEDATAPARSERS_H
