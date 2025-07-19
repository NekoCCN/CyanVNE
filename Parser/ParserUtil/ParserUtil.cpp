#include "ParserUtil.h"
#include <yaml-cpp/yaml.h>
#include <Parser/ParserException/ParserException.h>
#include <format>
#include <string>
#include <vector>
#include <map>
#include <ranges>

namespace cyanvne
{
    namespace parser
    {
        namespace util
        {
	        NodeOrBool getYamlNodeOrBool(const YAML::Node& node, const std::string& key, const std::string& parser_name)
	        {
                if (!node[key])
                {
                    throw exception::parserexception::ParserException(parser_name + " file(.yml) format error"
                        , std::format("{:s} file(.yml) contains no {:s} node.", parser_name, key));
                }

                if (node[key].IsScalar())
                {
                    try {
                        return node[key].as<bool>();
                    }
                    catch (const YAML::Exception&)
                    {
                        throw exception::parserexception::ParserException(parser_name + " file(.yml) format error"
                            , std::format("{:s} file(.yml) contains {:s} node, but the value is not a bool.", parser_name, key));
                    }
                }

                return node[key];
	        }

	        NodeOrBool getYamlNodeOrFalse(const YAML::Node& node, const std::string& key,
		        const std::string& parser_name)
	        {
                if (!node[key])
                {
                    throw exception::parserexception::ParserException(parser_name + " file(.yml) format error"
                        , std::format("{:s} file(.yml) contains no {:s} node.", parser_name, key));
                }

                if (node[key].IsScalar())
                {
                    try {
                        if (!node[key].as<bool>())
                        {
                            return false;
                        }
                        else
                        {
                            throw exception::parserexception::ParserException(parser_name + " file(.yml) format error"
                                , std::format("{:s} file(.yml) contains {:s} scalar node, but the value is not a false.", parser_name, key));
                        }
                    }
                    catch (const YAML::Exception&)
                    {
                        throw exception::parserexception::ParserException(parser_name + " file(.yml) format error"
                            , std::format("{:s} file(.yml) contains {:s} scalar node, but the value is not a bool.", parser_name, key));
                    }
                }

                return node[key];
	        }

	        bool checkIsBoolFalse(const YAML::Node& node, const std::string& parser_name)
	        {
                if (!node.IsScalar())
                {
                    return false;
                }

                try {
                    if (!node.as<bool>())
                    {
                        return true;
                    }
                }
                catch (const YAML::Exception&)
                {
                    return false;
                }

                return false;
	        }

	        bool checkIsBoolFalseIfTrueThrow(const YAML::Node& node, const std::string& parser_name)
	        {
                if (!node.IsScalar())
                {
                    return false;
                }

                try {
                    if (!node.as<bool>())
                    {
                        return true;
                    }
                    else
                    {
                        throw exception::parserexception::ParserException(parser_name + " file(.yml) format error"
                            , std::format("{:s} file(.yml) contains scalar node, but the value is not a false.", parser_name));
                    }
                }
                catch (const YAML::Exception&)
                {
                    return false;
                }
	        }

	        YAML::Node getYamlNodeElseThrow(const YAML::Node& node, const std::string& key,
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

                return sequence_node | std::views::transform([](const YAML::Node& em) { return em; })
                    | std::ranges::to<std::vector>();
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

                auto to_string_pair = [&](const YAML::const_iterator::value_type& yaml_pair) -> std::pair<std::string, YAML::Node>
                    {
                        const auto& yaml_key_node = yaml_pair.first;
                        const auto& yaml_value_node = yaml_pair.second;

                        try
                        {
                            if (!yaml_key_node.IsScalar())
                            {
                                throw YAML::Exception(yaml_key_node.Mark(),
                                    "Map key is not a scalar, cannot convert to string map key.");
                            }
                            return { yaml_key_node.as<std::string>(), yaml_value_node };
                        }
                        catch (const YAML::Exception& e)
                        {
                            throw exception::parserexception::ParserException(
                                parser_name + " file(.yml) format error",
                                std::format("Failed to convert a key in map '{:s}' (in {:s} file(.yml)) to string. YAML error: {:s}",
                                    key, parser_name, e.what()));
                        }
                    };

                return map_node | std::views::transform(to_string_pair) | std::ranges::to<std::map>();
            }

            std::map<std::string, YAML::Node> getMapSequenceAsNodeMapElseThrow(const YAML::Node& node, const std::string& key,
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

	            std::map<std::string, YAML::Node> result_map;

	            for (std::size_t i = 0; i < sequence_node.size(); ++i)
	            {
	                const YAML::Node& element = sequence_node[i];

	                if (!element.IsMap())
	                {
	                    throw exception::parserexception::ParserException(
                            parser_name + " file(.yml) format error",
                            std::format("Element at index {:d} in sequence '{:s}' (in {:s} file(.yml)) is not a map. Expected all elements to be maps.",
                                i, key, parser_name));
	                }

	                for (const auto& pair : element)
	                {
	                    try
	                    {
	                        const auto& yaml_key_node = pair.first;
	                        const auto& yaml_value_node = pair.second;

	                        if (!yaml_key_node.IsScalar())
	                        {
	                            throw YAML::Exception(yaml_key_node.Mark(),
                                    "Map key is not a scalar, cannot convert to string map key.");
	                        }

	                        std::string map_key = yaml_key_node.as<std::string>();

	                        if (result_map.contains(map_key))
	                        {
	                            throw exception::parserexception::ParserException(
                                    parser_name + " file(.yml) format error",
                                    std::format("Duplicate key '{:s}' found in map sequence '{:s}' (in {:s} file(.yml)).",
                                        map_key, key, parser_name));
	                        }

	                        result_map[map_key] = yaml_value_node;
	                    }
	                    catch (const YAML::Exception& e)
	                    {
	                        throw exception::parserexception::ParserException(
                                parser_name + " file(.yml) format error",
                                std::format("Failed to process element at index {:d} in map sequence '{:s}' (in {:s} file(.yml)). YAML error: {:s}",
                                    i, key, parser_name, e.what()));
	                    }
	                }
	            }

	            return result_map;
	        }

            std::map<std::string, YAML::Node> resolveMapSequenceAsNodeMapElseThrow(const YAML::Node& node,
                const std::string& parser_name, const std::string& key)
            {
	        	const YAML::Node& sequence_node = node;

	            if (!sequence_node.IsSequence())
	            {
	                throw exception::parserexception::ParserException(
                        parser_name + " file(.yml) format error",
                        std::format("The node in {:s} file(.yml) is not a sequence. Expected sequence.",
                           key, parser_name));
	            }

	            std::map<std::string, YAML::Node> result_map;

	            for (std::size_t i = 0; i < sequence_node.size(); ++i)
	            {
	                const YAML::Node& element = sequence_node[i];

	                if (!element.IsMap())
	                {
	                    throw exception::parserexception::ParserException(
                            parser_name + " file(.yml) format error",
                            std::format("Element at index {:d} in sequence '{:s}' (in {:s} file(.yml)) is not a map. Expected all elements to be maps.",
                                i, key, parser_name));
	                }

	                for (const auto& pair : element)
	                {
	                    try
	                    {
	                        const auto& yaml_key_node = pair.first;
	                        const auto& yaml_value_node = pair.second;

	                        if (!yaml_key_node.IsScalar())
	                        {
	                            throw YAML::Exception(yaml_key_node.Mark(),
                                    "Map key is not a scalar, cannot convert to string map key.");
	                        }

	                        std::string map_key = yaml_key_node.as<std::string>();

	                        if (result_map.contains(map_key))
	                        {
	                            throw exception::parserexception::ParserException(
                                    parser_name + " file(.yml) format error",
                                    std::format("Duplicate key '{:s}' found in map sequence '{:s}' (in {:s} file(.yml)).",
                                        map_key, key, parser_name));
	                        }

	                        result_map[map_key] = yaml_value_node;
	                    }
	                    catch (const YAML::Exception& e)
	                    {
	                        throw exception::parserexception::ParserException(
                                parser_name + " file(.yml) format error",
                                std::format("Failed to process element at index {:d} in map sequence '{:s}' (in {:s} file(.yml)). YAML error: {:s}",
                                    i, key, parser_name, e.what()));
	                    }
	                }
	            }

	            return result_map;
            }
        }
    }
}