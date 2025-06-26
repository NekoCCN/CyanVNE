#pragma once
#include <Parser/Framework/Framework.h>
#include <Parser/ParserUtil/ParserUtil.h>
#include "../AppSettings.h"

namespace cyanvne
{
    namespace parser
    {
        namespace appsettings
        {
            class LoggerConfigParser : public INodeParser
            {
            public:
                std::unique_ptr<ParsedNodeData> parse(
                    const YAML::Node& node,
                    const NodeParserRegistry& registry) const override;
                std::string getParsableNodeType() const override;
            };

            class DefaultLoadingConfigParser : public INodeParser
            {
            public:
                std::unique_ptr<ParsedNodeData> parse(const YAML::Node& node, const NodeParserRegistry& registry) const override;
                std::string getParsableNodeType() const override;
            };

            class WindowConfigParser : public INodeParser
            {
            public:
                std::unique_ptr<ParsedNodeData> parse(const YAML::Node& node, const NodeParserRegistry& registry) const override;
                std::string getParsableNodeType() const override;
            };

            class VsyncConfigParser : public INodeParser
            {
            public:
                std::unique_ptr<ParsedNodeData> parse(const YAML::Node& node, const NodeParserRegistry& registry) const override;
                std::string getParsableNodeType() const override;
            };

            class LanguageConfigParser : public INodeParser
            {
            public:
                std::unique_ptr<ParsedNodeData> parse(const YAML::Node& node, const NodeParserRegistry& registry) const override;
                std::string getParsableNodeType() const override;
            };

            class CachingConfigParser : public INodeParser
            {
            public:
                std::unique_ptr<ParsedNodeData> parse(const YAML::Node& node, const NodeParserRegistry& registry) const override;
                std::string getParsableNodeType() const override;
            };

            class AppCachingConfigParser : public INodeParser
            {
            public:
                std::unique_ptr<ParsedNodeData> parse(const YAML::Node& node, const NodeParserRegistry& registry) const override;
                std::string getParsableNodeType() const override;
            };

            class AppNodeParser : public INodeParser
            {
            public:
                std::unique_ptr<ParsedNodeData> parse(
                    const YAML::Node& node,
                    const NodeParserRegistry& registry) const override;
                std::string getParsableNodeType() const override;
            };
        }
    }
}
