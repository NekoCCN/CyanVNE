//
// Created by Ext Culith on 2025/7/3.
//

#ifndef COMPONENTPARSERS_H
#define COMPONENTPARSERS_H
#include <entt/entt.hpp>
#include <yaml-cpp/yaml.h>
#include "Parser/Framework/Framework.h"

namespace cyanvne::parser::ecs
{
    class IComponentEmplacer
    {
    public:
        virtual ~IComponentEmplacer() = default;
        virtual void emplace_to(entt::registry& registry, entt::entity entity) const = 0;
    };

    template<typename T>
    class ComponentEmplacer : public IComponentEmplacer
    {
    private:
        T component_data_;
    public:
        explicit ComponentEmplacer(T data) : component_data_(std::move(data))
        {  }
        void emplace_to(entt::registry& registry, entt::entity entity) const override
        {
            registry.emplace<T>(entity, component_data_);
        }
    };

    class IdentifierComponentParser : public INodeParser
    {
    public:
        [[nodiscard]] std::unique_ptr<ParsedNodeData> parse(const YAML::Node& node, const NodeParserRegistry& main_registry) const override;
        [[nodiscard]] std::string getParsableNodeType() const override
        {
            return "identifier";
        }
    };

    class TransformComponentParser : public INodeParser
    {
    public:
        [[nodiscard]] std::unique_ptr<ParsedNodeData> parse(const YAML::Node& node, const NodeParserRegistry& main_registry) const override;
        [[nodiscard]] std::string getParsableNodeType() const override
        {
            return "transform";
        }
    };

    class LayoutComponentParser : public INodeParser
    {
    public:
        [[nodiscard]] std::unique_ptr<ParsedNodeData> parse(const YAML::Node& node, const NodeParserRegistry& main_registry) const override;
        [[nodiscard]] std::string getParsableNodeType() const override
        {
            return "layout";
        }
    };

    class SpriteComponentParser : public INodeParser
    {
    public:
        [[nodiscard]] std::unique_ptr<ParsedNodeData> parse(const YAML::Node& node, const NodeParserRegistry& main_registry) const override;
        [[nodiscard]] std::string getParsableNodeType() const override
        {
            return "sprite";
        }
    };

    class SpriteAnimationComponentParser : public INodeParser
    {
    public:
        [[nodiscard]] std::unique_ptr<ParsedNodeData> parse(const YAML::Node& node, const NodeParserRegistry& main_registry) const override;
        [[nodiscard]] std::string getParsableNodeType() const override
        {
            return "sprite_animation";
        }
    };

    class ClickableComponentParser : public INodeParser
    {
    public:
        [[nodiscard]] std::unique_ptr<ParsedNodeData> parse(const YAML::Node& node, const NodeParserRegistry& main_registry) const override;
        [[nodiscard]] std::string getParsableNodeType() const override
        {
            return "clickable";
        }
    };

    class KeyFocusComponentParser : public INodeParser
    {
    public:
        [[nodiscard]] std::unique_ptr<ParsedNodeData> parse(const YAML::Node& node, const NodeParserRegistry& main_registry) const override;
        [[nodiscard]] std::string getParsableNodeType() const override
        {
            return "key_focus";
        }
    };

    class ScrollableComponentParser : public INodeParser
    {
    public:
        [[nodiscard]] std::unique_ptr<ParsedNodeData> parse(const YAML::Node& node, const NodeParserRegistry& main_registry) const override;
        [[nodiscard]] std::string getParsableNodeType() const override
        {
            return "scrollable";
        }
    };

    class IdentifierComponent : public INodeParser
    {
    public:
        [[nodiscard]] std::unique_ptr<ParsedNodeData> parse(const YAML::Node& node, const NodeParserRegistry& main_registry) const override;
        [[nodiscard]] std::string getParsableNodeType() const override
        {
            return "identifier";
        }
    };
}

#endif // COMPONENTPARSERS_H