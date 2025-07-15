//
// Created by Ext Culith on 2025/7/8.
//

#ifndef COMMANDSPARSER_H
#define COMMANDSPARSER_H
#include <Parser/Framework/Framework.h>
#include <Parser/Ecs/ComponentParsers.h>
#include <Parser/ParserUtil/ParserUtil.h>

namespace cyanvne::parser
{
    class CommandsParser : public INodeParser
    {
    private:
        std::shared_ptr<NodeParserRegistry> component_parser_registry_;
    public:
        explicit CommandsParser(std::shared_ptr<NodeParserRegistry> registry)
            : component_parser_registry_(std::move(registry))
        {  }

        std::unique_ptr<ParsedNodeData> parse(const YAML::Node& node, const NodeParserRegistry&) const override
        {
            if (node.IsNull() || !node.IsSequence())
            {
                throw exception::parserexception::ParserException("Commands Parser Error", "Expected a sequence of commands.");
            }

            for (const YAML::Node& command : node)
            {
                if (!command.IsMap())
                {
                    throw exception::parserexception::ParserException("Commands Parser Error",
                        "Each command must be a map with a 'type' field.");
                }

                for (const auto& key : command)
                {
                    if (key.first.as<std::string>() == "type")
                    {
                    }
                }
            }

            return std::make_unique<ParsedNodeData>(entity_data, getParsableNodeType());
        }

        std::string getParsableNodeType() const override
        {
            return "commands";
        }
    };
}


#endif //COMMANDSPARSER_H
