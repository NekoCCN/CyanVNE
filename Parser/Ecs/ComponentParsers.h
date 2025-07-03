//
// Created by Ext Culith on 2025/7/3.
//

#ifndef COMPONENTPARSERS_H
#define COMPONENTPARSERS_H
#include <entt/entt.hpp>
#include <yaml-cpp/yaml.h>
#include <Parser/ParserException/ParserException.h>

#include "Parser/Framework/Framework.h"

namespace cyanvne::parser::ecs
{
    class IComponentEmplacer
    {
    public:
        virtual ~IComponentEmplacer() = default;
        virtual void emplace_to(entt::registry& registry, entt::entity entity) const = 0;
    };
    class IComponentParser
    {
    public:
        virtual ~IComponentParser() = default;
        virtual std::unique_ptr<IComponentEmplacer> parse(const YAML::Node& node) const = 0;
    };

    class ComponentParserRegistry
    {
    private:
        std::unordered_map<std::string, std::shared_ptr<IComponentParser>> parsers_;
    public:
        void registerParser(const std::string& name, std::shared_ptr<IComponentParser> parser)
        {
            parsers_[name] = std::move(parser);
        }
        std::unique_ptr<IComponentEmplacer> parseComponent(const std::string& name, const YAML::Node& node) const
        {
            if (auto it = parsers_.find(name); it != parsers_.end())
            {
                return it->second->parse(node);
            }
            throw exception::parserexception::ParserException("Component Parser Error", "No parser registered for component type: " + name);
        }
    };

    template<typename T>
    class ComponentEmplacer : public IComponentEmplacer
    {
        T component_data_;
    public:
        ComponentEmplacer(T data) : component_data_(std::move(data))
        {  }
        void emplace_to(entt::registry& registry, entt::entity entity) const override
        {
            registry.emplace<T>(entity, component_data_);
        }
    };

    class TransformComponentParser : public IComponentParser
    {
    public:
        std::unique_ptr<IComponentEmplacer> parse(const YAML::Node& node, const NodeParserRegistry& main_registry) const override;
    };

    class LayoutComponentParser : public IComponentParser
    {
    public:
        std::unique_ptr<IComponentEmplacer> parse(const YAML::Node& node, const NodeParserRegistry& main_registry) const override;
    };

    class SpriteComponentParser : public IComponentParser
    {
    public:
        std::unique_ptr<IComponentEmplacer> parse(const YAML::Node& node, const NodeParserRegistry& main_registry) const override;
    };

    class SpriteAnimationComponentParser : public IComponentParser
    {
    public:
        std::unique_ptr<IComponentEmplacer> parse(const YAML::Node& node, const NodeParserRegistry& main_registry) const override;
    };

    class ClickableComponentParser : public IComponentParser
    {
    public:
        std::unique_ptr<IComponentEmplacer> parse(const YAML::Node& node, const NodeParserRegistry& main_registry) const override;
    };

    class KeyFocusComponentParser : public IComponentParser
    {
    public:
        std::unique_ptr<IComponentEmplacer> parse(const YAML::Node& node, const NodeParserRegistry& main_registry) const override;
    };

    class ScrollableComponentParser : public IComponentParser
    {
    public:
        std::unique_ptr<IComponentEmplacer> parse(const YAML::Node& node, const NodeParserRegistry& main_registry) const override;
    };
}
