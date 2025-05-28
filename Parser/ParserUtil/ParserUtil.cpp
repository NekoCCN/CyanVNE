#include "ParserUtil.h"
#include <yaml-cpp/yaml.h>
#include <Parser/ParserException/ParserException.h>
#include <format>
#include <string>
#include <vector>
#include <map>

namespace cyanvne
{
    namespace parser
    {
        namespace util
        {
            const YAML::Node& getYamlNodeElseThrow(const YAML::Node& node, const std::string& key,
                const std::string& parser_name)
            {
                if (!node[key])
                {
                    throw exception::parserexception::ParserException(parser_name + " file(.yml) format error"
                        , std::format("{:s} file(.yml) contains no {:s} node.", parser_name, key));
                }
                return node[key];
            }

            std::vector<YAML::Node> getSequenceAsNodeVectorElseThrow(const YAML::Node& node, const std::string& key,
                const std::string& parser_name)
            {
                const YAML::Node& sequence_node = getYamlNodeElseThrow(node, key, parser_name);

                if (!sequence_node.IsSequence())
                {
                    throw exception::parserexception::ParserException(
                        parser_name + " file(.yml) format error",
                        std::format("The node '{:s}' in {:s} file(.yml) is not a sequence. Expected sequence.",
                            key, parser_name));
                }

                std::vector<YAML::Node> result;
                result.reserve(sequence_node.size());
                for (const auto& item_node : sequence_node)
                {
                    result.push_back(item_node);
                }
                return result;
            }

            std::map<std::string, YAML::Node> getMapAsNodeMapElseThrow(const YAML::Node& node, const std::string& key,
                const std::string& parser_name)
            {
                const YAML::Node& map_node = getYamlNodeElseThrow(node, key, parser_name);

                if (!map_node.IsMap())
                {
                    throw exception::parserexception::ParserException(
                        parser_name + " file(.yml) format error",
                        std::format("The node '{:s}' in {:s} file(.yml) is not a map. Expected map.",
                            key, parser_name));
                }

                std::map<std::string, YAML::Node> result;
                for (YAML::const_iterator it = map_node.begin(); it != map_node.end(); ++it)
                {
                    std::string map_item_key;
                    try
                    {
                        if (!it->first.IsScalar())
                        {
                            throw YAML::Exception(it->first.Mark(), std::string("Map key is not a scalar, cannot convert to string map key. Node content preview: '") + YAML::Dump(it->first).substr(0, 50) + "'.");
                        }
                        map_item_key = it->first.as<std::string>();
                    }
                    catch (const YAML::Exception& e)
                    {
                        throw exception::parserexception::ParserException(
                            parser_name + " file(.yml) format error",
                            std::format("Failed to convert a key in map '{:s}' (in {:s} file(.yml)) to string. "
                                "YAML error: {:s}",
                                key, parser_name, e.what()));
                    }

                    result.emplace(map_item_key, it->second);
                }
                return result;
            }
        }
    }
}