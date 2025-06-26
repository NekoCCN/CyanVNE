#include "Framework.h"
#include <format>
#include <ranges>

namespace cyanvne
{
    namespace parser
    {
        void NodeParserRegistry::registerParser(const std::shared_ptr<INodeParser>& parser)
        {
            if (!parser)
            {
                throw exception::IllegalArgumentException("Cannot register a null parser.");
            }
            std::string keyOrType = parser->getParsableNodeType();
            if (keyOrType.empty())
            {
                throw exception::IllegalArgumentException("Parser's parsable node type/key cannot be empty.");
            }
            parsers_by_key_or_type_[keyOrType] = parser;
        }

        void NodeParserRegistry::registerDefaultParser(const std::shared_ptr<INodeParser>& parser)
        {
            if (!parser)
            {
                throw exception::IllegalArgumentException("Cannot register a null default parser.");
            }
            default_parser_ = parser;
        }

        std::shared_ptr<cyanvne::parser::INodeParser> NodeParserRegistry::getParser(
            const std::string& keyOrType) const
        {
            auto it = parsers_by_key_or_type_.find(keyOrType);
            if (it != parsers_by_key_or_type_.end())
            {
                return it->second;
            }
            return default_parser_;
        }

        std::unique_ptr<cyanvne::parser::ParsedNodeData> NodeParserRegistry::parseNodeByKeyOrType(
            const std::string& keyOrTypeIdentifier, const YAML::Node& ymlNode) const
        {
	        if (std::shared_ptr<INodeParser> parser_to_use = getParser(keyOrTypeIdentifier))
            {
                try
                {
                    return parser_to_use->parse(ymlNode, *this);
                }
                catch (const exception::parserexception::ParserException&)
                {
                    throw;
                }
                catch (const YAML::Exception& e)
                {
                    throw exception::parserexception::ParserException(
                        std::format("YAML error during parsing using identifier '{}'", keyOrTypeIdentifier),
                        std::format("Parser (registered as '{}') encountered a YAML exception: {}",
                            keyOrTypeIdentifier, e.what())
                    );
                }
                catch (const std::exception& e)
                {
                    throw exception::parserexception::ParserException(
                        std::format("Standard error during parsing using identifier '{}'", keyOrTypeIdentifier),
                        std::format("Parser (registered as '{}') encountered an std::exception: {}",
                            keyOrTypeIdentifier, e.what())
                    );
                }
                catch (...)
                {
                    throw exception::parserexception::ParserException(
                        std::format("Unknown error during parsing using identifier '{}'", keyOrTypeIdentifier),
                        std::format("Parser (registered as '{}') encountered an unknown exception.",
                            keyOrTypeIdentifier)
                    );
                }
            }
            else
            {
                throw exception::parserexception::ParserException(
                    std::format("Parser not found for identifier '{}'", keyOrTypeIdentifier),
                    std::format("No parser registered (and no default parser available) for identifier: '{}'.", keyOrTypeIdentifier)
                );
            }
        }

        std::unique_ptr<cyanvne::parser::ParsedNodeData> NodeParserRegistry::parseNodeFromTypeField(
            const YAML::Node& ymlNode, const std::string& type_field_name) const
        {
            const YAML::Node typeNode = ymlNode[type_field_name];
            if (!typeNode || !typeNode.IsScalar())
            {
                if (default_parser_)
                {
                    try
                    {
                        return default_parser_->parse(ymlNode, *this);
                    }
                    catch (const exception::parserexception::ParserException&)
                    {
                        throw;
                    }
                    catch (const std::exception& e)
                    {
                        throw exception::parserexception::ParserException(
                            std::format("Error while default-parsing node lacking/invalid '{}' field", type_field_name),
                            std::format("Default parser failed: {}", e.what())
                        );
                    }
                }
                else
                {
                    throw exception::parserexception::ParserException(
                        "Missing or invalid type field for typed parsing",
                        std::format("YAML node is missing a scalar '{}' field, and no default parser is registered.", type_field_name)
                    );
                }
            }
            std::string nodeTypeIdentifier = typeNode.as<std::string>();
            return parseNodeByKeyOrType(nodeTypeIdentifier, ymlNode);
        }

        std::map<std::string, std::unique_ptr<cyanvne::parser::ParsedNodeData>> NodeParserRegistry::parseMapNodeEntries(const YAML::Node& map_node) const
        {
            if (!map_node.IsMap())
            {
                throw exception::parserexception::ParserException(
                    "Invalid node for map entry parsing", "Expected a map node."
                );
            }

            auto to_string_pair = [this](const YAML::const_iterator::value_type& it) -> 
                std::pair<std::string, std::unique_ptr<cyanvne::parser::ParsedNodeData>>
                {
                    std::string key_name;
                    try
                    {
                        if (!it.first.IsScalar())
                        {
                            throw YAML::Exception(it.first.Mark(), "Map key is not a scalar.");
                        }
                        key_name = it.first.as<std::string>();
                    }
                    catch (const YAML::Exception& e)
                    {
                        throw exception::parserexception::ParserException(
                            "Invalid map key",
                            std::format("Failed to convert map key to string: {}", e.what())
                        );
                    }

                    const YAML::Node& value_node = it.second;
                    return { key_name, parseNodeByKeyOrType(key_name, value_node) };
                };

            return map_node | std::views::transform(to_string_pair) | std::ranges::to<std::map>();
        }

        std::vector<std::unique_ptr<ParsedNodeData>> NodeParserRegistry::parseSequenceNodeItems(const YAML::Node& sequence_node, const std::string& type_field_name) const
        {
            if (!sequence_node.IsSequence())
            {
                throw exception::parserexception::ParserException(
                    "Invalid node for sequence item parsing", "Expected a sequence node."
                );
            }

            return sequence_node | std::views::transform([this, type_field_name](const YAML::Node& item_node)
                {
                    return parseNodeFromTypeField(item_node, type_field_name);
                }) | std::ranges::to<std::vector>();
        }
    }
}
