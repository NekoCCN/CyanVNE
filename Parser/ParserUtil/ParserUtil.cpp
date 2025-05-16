#include "ParserUtil.h"

const YAML::Node& cyanvne::parser::util::getYamlNodeElseThrow(const YAML::Node& node, const std::string& key,
                                                              const std::string& parser_name)
{
	if (!node[key])
	{
		throw exception::parserexception::ParserException(parser_name + " file(.yml) format error"
			, std::format("{:s} file(.yml) contains no {:s} node.", parser_name, key));
	}

	return node[key];
}
