#pragma once
#include <memory>
#include <string>
#include <map>
#include <any>
#include <format>
#include <yaml-cpp/yaml.h>
#include "Parser/ParserException/ParserException.h"
#include <Parser/ParserUtil/ParserUtil.h>

namespace cyanvne
{
    namespace parser
    {
	    class NodeParserRegistry;

	    class ParsedNodeData
        {
        private:
            std::any data_;
            std::string original_node_type_;
        public:
            template<typename T> ParsedNodeData(T&& data, std::string nodeType)
                : data_(std::forward<T>(data)), original_node_type_(std::move(nodeType))
            {  }

            ParsedNodeData(const ParsedNodeData&) = delete;
            ParsedNodeData& operator=(const ParsedNodeData&) = delete;
            ParsedNodeData(ParsedNodeData&&) = delete;
            ParsedNodeData& operator=(ParsedNodeData&&) = delete;

            const std::string& getOriginalNodeType() const
            {
                return original_node_type_;
            }

            template<typename T> const T* getAs() const
            {
                return std::any_cast<T>(&data_);
            }

            template<typename T> T* getAs()
            {
                return std::any_cast<T>(&data_);
            }

            const std::type_info& getHeldType() const
            {
                return data_.type();
            }

            virtual ~ParsedNodeData() = default;
        };

        class INodeParser
        {
        protected:
            INodeParser() = default;
        public:
            INodeParser(const INodeParser&) = delete;
            INodeParser& operator=(const INodeParser&) = delete;
            INodeParser(INodeParser&&) = delete;
            INodeParser& operator=(INodeParser&&) = delete;

            virtual std::unique_ptr<ParsedNodeData> parse(
                const YAML::Node& node,
                const NodeParserRegistry& registry) const = 0;

            virtual std::string getParsableNodeType() const = 0;

            virtual ~INodeParser() = default;
        };

        class NodeParserRegistry
        {
        private:
            std::map<std::string, std::shared_ptr<INodeParser>> parsers_by_key_or_type_;
            std::shared_ptr<INodeParser> default_parser_;
        public:
            NodeParserRegistry() = default;

            NodeParserRegistry(const NodeParserRegistry&) = default;
            NodeParserRegistry& operator=(const NodeParserRegistry&) = default;
            NodeParserRegistry(NodeParserRegistry&&) noexcept = default;
            NodeParserRegistry& operator=(NodeParserRegistry&&) noexcept = default;

            void registerParser(const std::shared_ptr<INodeParser>& parser);

            void registerDefaultParser(const std::shared_ptr<INodeParser>& parser);

            std::shared_ptr<INodeParser> getParser(const std::string& keyOrType) const;

            std::unique_ptr<ParsedNodeData> parseNodeByKeyOrType(
                const std::string& keyOrTypeIdentifier,
                const YAML::Node& ymlNode) const;

            std::unique_ptr<ParsedNodeData> parseNodeFromTypeField(
                const YAML::Node& ymlNode,
                const std::string& type_field_name = "type") const;
            std::map<std::string, std::unique_ptr<cyanvne::parser::ParsedNodeData>> parseMapNodeEntries(
	            const YAML::Node& map_node) const;
            std::vector<std::unique_ptr<cyanvne::parser::ParsedNodeData>> parseSequenceNodeItems(
	            const YAML::Node& sequence_node, const std::string& type_field_name) const;

            template<typename T>
            T parseSubNodeAs(const YAML::Node& parentNode, const std::string& key,
                const std::string& parentParserName) const
            {
                const auto& sub_node = util::getYamlNodeElseThrow(parentNode, key, parentParserName);

                if (auto parsed_data = this->parseNodeByKeyOrType(key, sub_node))
                {
                    if (const auto* config = parsed_data->getAs<T>())
                    {
                        return *config;
                    }
                }
                throw exception::parserexception::ParserException(
                    "Internal Parser Error",
                    std::format("Failed to parse or cast sub-node '{}' to the requested type.", key)
                );
            }

            template<typename T>
            T useParserPraseSubNodeAs(const YAML::Node& parentNode, const std::string& parser_type, 
                const std::string& key,
                const std::string& parentParserName) const
            {
                const auto& sub_node = util::getYamlNodeElseThrow(parentNode, key, parentParserName);

                if (auto parsed_data = this->parseNodeByKeyOrType(parser_type, sub_node))
                {
                    if (const auto* config = parsed_data->getAs<T>())
                    {
                        return *config;
                    }
                }
                throw exception::parserexception::ParserException(
                    "Internal Parser Error",
                    std::format("Failed to parse or cast sub-node '{}' to the requested type.", key)
                );
            }

            virtual ~NodeParserRegistry() = default;
        };
    }
}
