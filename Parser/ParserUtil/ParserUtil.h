#pragma once
#include <yaml-cpp/yaml.h>
#include <Parser/ParserException/ParserException.h>
#include <format>

namespace cyanvne
{
	namespace parser
	{
		namespace util
		{
			// parser_name param for exception message
			const YAML::Node& getYamlNodeElseThrow(const YAML::Node& node, const std::string& key,
				const std::string& parser_name);

			// parser_name param for exception message
			template <typename T> T getScalarNodeElseThrow(const YAML::Node& node, const std::string& key,
				const std::string& parser_name)
			{
				if (!node[key])
				{
					throw exception::parserexception::ParserException(parser_name + " file(.yml) format error"
						, std::format("{:s} file(.yml) contains no {:s} node.", parser_name, key));
				}

				if (!node[key].IsScalar())
				{
					throw exception::parserexception::ParserException(parser_name + " file(.yml) format error",
						std::format("The {:s} node in {:s} file(.yml) have INVAILD TYPE.",
							key, parser_name));
				}

				try {
					return node[key].as<T>();
				}
				catch (const YAML::Exception& e)
				{
					throw exception::parserexception::ParserException(parser_name + " file(.yml) format error",
						std::format("The {:s} node in {:s} file(.yml) have INVAILD TYPE.",
							key, parser_name));
				}
			}
		}
	}
}