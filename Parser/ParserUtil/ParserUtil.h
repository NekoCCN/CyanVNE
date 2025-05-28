#pragma once
#include <yaml-cpp/yaml.h>
#include <Parser/ParserException/ParserException.h>
#include <format>
#include <vector>
#include <string>
#include <map>

namespace cyanvne
{
    namespace parser
    {
        namespace util
        {
            const YAML::Node& getYamlNodeElseThrow(const YAML::Node& node, const std::string& key,
                const std::string& parser_name);

            std::vector<YAML::Node> getSequenceAsNodeVectorElseThrow(const YAML::Node& parent_node, const std::string& key,
                const std::string& parser_name);

            std::map<std::string, YAML::Node> getMapAsNodeMapElseThrow(const YAML::Node& parent_node, const std::string& key,
                const std::string& parser_name);

            template <typename T>
            T getScalarNodeElseThrow(const YAML::Node& node, const std::string& key,
                const std::string& parser_name)
            {
                const YAML::Node& target_node = getYamlNodeElseThrow(node, key, parser_name);

                if (!target_node.IsScalar())
                {
                    throw exception::parserexception::ParserException(
                        parser_name + " file(.yml) format error",
                        std::format("The node '{:s}' in {:s} file(.yml) is not a scalar. Expected scalar.",
                            key, parser_name));
                }
                try
                {
                    return target_node.as<T>();
                }
                catch (const YAML::Exception& e)
                {
                    throw exception::parserexception::ParserException(
                        parser_name + " file(.yml) format error",
                        std::format("Failed to convert scalar node '{:s}' in {:s} file(.yml) to the desired type. YAML error: {:s}",
                            key, parser_name, e.what()));
                }
            }

            template <typename T>
            std::vector<T> getSequenceAsVectorElseThrow(const YAML::Node& node, const std::string& key,
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

                std::vector<T> result;
                result.reserve(sequence_node.size());

                for (std::size_t i = 0; i < sequence_node.size(); ++i)
                {
                    const YAML::Node& item_node = sequence_node[i];
                    try
                    {
                        result.push_back(item_node.as<T>());
                    }
                    catch (const YAML::Exception& e)
                    {
                        throw exception::parserexception::ParserException(
                            parser_name + " file(.yml) format error",
                            std::format("Failed to convert element at index {:d} in sequence '{:s}' (in {:s} file(.yml)) "
                                "to the desired type, or element types are not uniform. YAML error: {:s}",
                                i, key, parser_name, e.what()));
                    }
                }
                return result;
            }

            template <typename T_Value>
            std::map<std::string, T_Value> getMapAsStdMapElseThrow(const YAML::Node& node, const std::string& key,
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

                std::map<std::string, T_Value> result;
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

                    const YAML::Node& value_node = it->second;
                    try
                    {
                        result.emplace(map_item_key, value_node.as<T_Value>());
                    }
                    catch (const YAML::Exception& e)
                    {
                        throw exception::parserexception::ParserException(
                            parser_name + " file(.yml) format error",
                            std::format("Failed to convert value for key '{:s}' in map '{:s}' (in {:s} file(.yml)) "
                                "to the desired type, or value types are not uniform. YAML error: {:s}",
                                map_item_key, key, parser_name, e.what()));
                    }
                }
                return result;
            }
        }
    }
}