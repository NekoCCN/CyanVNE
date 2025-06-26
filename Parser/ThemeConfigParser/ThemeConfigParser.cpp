#include <Parser/Framework/Framework.h>
#include <Parser/ThemeConfig/ThemeConfig.h>
#include "ThemeConfigParser.h"

namespace cyanvne::parser::theme
{
	std::unique_ptr<ParsedNodeData> AreaScaleParser::parse(const YAML::Node& node, const NodeParserRegistry& registry) const
	{
		AreaScaleData data;

		if (!node.IsSequence())
		{
			throw exception::parserexception::ParserException("AreaScaleParser Error",
				"area_scale is not a sequence.");
		}

		data.x = util::getScalarNodeElseThrow<double>(node, "x", getParsableNodeType());
		data.y = util::getScalarNodeElseThrow<double>(node, "y", getParsableNodeType());
		
		if (!node["h"] && !node["w"])
		{
			throw exception::parserexception::ParserException("AreaScaleParser Error",
				"area_scale node must have either w or h field");
		}
		
		if (node["w"])
		{
			data.w = util::getScalarNodeElseThrow<double>(node, "w",
				getParsableNodeType());
		}
		if (node["h"])
		{
			data.h = util::getScalarNodeElseThrow<double>(node, "h", getParsableNodeType());
		}
		return std::make_unique<ParsedNodeData>(std::move(data), getParsableNodeType());
	}
	std::string AreaScaleParser::getParsableNodeType() const
	{
		return "area_scale";
	}


	std::unique_ptr<ParsedNodeData> AnimationParser::parse(const YAML::Node& node, const NodeParserRegistry& registry) const
	{
		AnimationData data;
		data.ms_per_frame = util::getScalarNodeElseThrow<int32_t>(node, "ms_per_frame", getParsableNodeType());
		const auto& sheet_node = util::getYamlNodeElseThrow(node, "sprite_sheet", getParsableNodeType());

		if (!sheet_node.IsSequence())
		{
			throw exception::parserexception::ParserException("Animation Parser Error",
				"sprite_sheet is not a sequence.");
		}

		for (const auto& sprite_node : sheet_node)
		{
			if (!sprite_node.IsMap())
				continue;
			data.sprite_sheet.push_back({
				util::getScalarNodeElseThrow<uint32_t>(sprite_node, "x", getParsableNodeType()),
				util::getScalarNodeElseThrow<uint32_t>(sprite_node, "y", getParsableNodeType()),
				util::getScalarNodeElseThrow<uint32_t>(sprite_node, "w", getParsableNodeType()),
				util::getScalarNodeElseThrow<uint32_t>(sprite_node, "h", getParsableNodeType())
				});
		}
		return std::make_unique<ParsedNodeData>(std::move(data), getParsableNodeType());
	}
	std::string AnimationParser::getParsableNodeType() const
	{
		return "animation";
	}


	std::unique_ptr<ParsedNodeData> ThemeResourceParser::parse(const YAML::Node& node, const NodeParserRegistry& registry) const
	{
		ThemeResource resource;

		if (util::checkIsBoolFalseIfTrueThrow(node, getParsableNodeType()))
		{
			return std::make_unique<ParsedNodeData>(std::move(resource), getParsableNodeType());
		}

		if (const auto& animation_node = node["animation"])
		{
			if (!animation_node.IsScalar() || animation_node.as<std::string>() != "false")
			{
				resource.animation.emplace(registry.parseSubNodeAs<AnimationData>(node,
					"animation", getParsableNodeType()));
			}
		}

		if (node["area_scale"])
		{
			resource.area_scale.emplace(registry.parseSubNodeAs<AreaScaleData>(node,
				"area_scale", getParsableNodeType()));
		}

		return std::make_unique<ParsedNodeData>(std::move(resource), getParsableNodeType());
	}
	std::string ThemeResourceParser::getParsableNodeType() const
	{
		return "theme_resource";
	}


	std::unique_ptr<ParsedNodeData> ThemeParser::parse(const YAML::Node& node, const NodeParserRegistry& registry) const
	{
		ThemeConfig config;
		config.name = util::getScalarNodeElseThrow<std::string>(node, "name", getParsableNodeType());

		util::NodeOrBool built_in_font_val = util::getYamlNodeOrFalse(node, "built_in_font",
			getParsableNodeType());

		try
		{
			config.enable_built_in_font = std::get<bool>(built_in_font_val);
		}
		catch (const std::bad_variant_access&)
		{
			config.enable_built_in_font = true;
		}

		for (const std::string& x : REQUIRED_KEY_)
		{
            YAML::Node sub_theme_node = util::getYamlNodeElseThrow(node, x, getParsableNodeType());

			auto resource = registry.useParserPraseSubNodeAs<ThemeResource>(node,
				"theme_resource", x, getParsableNodeType());
			resource.key = x;
			config.resources.emplace(x, std::move(resource));
		}

		return std::make_unique<ParsedNodeData>(std::move(config), getParsableNodeType());
	}
	std::string ThemeParser::getParsableNodeType() const
	{
		return "theme_config";
	}

	std::unique_ptr<ParsedNodeData> ThemeGeneratorParser::parse(const YAML::Node& node,
	                                                            const NodeParserRegistry& registry) const
	{
		ThemeGeneratorConfig config;

		util::NodeOrBool built_in_font_val = util::getYamlNodeOrFalse(node, "built_in_font",
			getParsableNodeType());

		try {
			if (std::get<bool>(built_in_font_val))
			{
				throw exception::parserexception::ParserException("ThemeGeneratorParser Error",
					"built_in_font is true, but no built_in_font_path is provided.");
			}
		}
		catch (const std::exception&)
		{
			config.resources.emplace("built_in_font", util::getScalarNodeElseThrow<std::string>(std::get<YAML::Node>(built_in_font_val),
				"path", getParsableNodeType()));
		}

		for (const std::string& x : REQUIRED_KEY_)
		{
			util::NodeOrBool val = util::getYamlNodeOrFalse(node, x, getParsableNodeType());

			try
			{
				if (std::get<bool>(val))
				{
					throw exception::parserexception::ParserException("ThemeGeneratorParser Error",
						"built_in_font is true, but no built_in_font_path is provided.");
				}
			}
			catch (const std::exception&)
			{
				config.resources.emplace(x, util::getScalarNodeElseThrow<std::string>(std::get<YAML::Node>(val),
					"path", getParsableNodeType()));
			}
		}

		return std::make_unique<ParsedNodeData>(std::move(config), getParsableNodeType());
	}

	std::string ThemeGeneratorParser::getParsableNodeType() const
	{
		return "theme_generator_config";
	}
}
