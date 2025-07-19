#pragma once
#include <Parser/Framework/Framework.h>
#include <Parser/AppSettings/AppSettingsParsers/AppSettingsParsers.h>
#include <Resources/StreamUniversalImpl/StreamUniversalImpl.h>

#include "Parser/GameDataParsers/GameDataParsers.h"
#include "Parser/ThemeConfig/ThemeConfig.h"
#include "Parser/ThemeConfigParser/ThemeConfigParser.h"

namespace cyanvne
{
    namespace parser
    {
        namespace factories
        {
	        inline NodeParserRegistry createAppSettingsRegistry()
	        {
	        	using namespace appsettings;
	        	NodeParserRegistry registry;
	        	registry.registerParser(std::make_shared<AppNodeParser>());
	        	registry.registerParser(std::make_shared<WindowConfigParser>());
	        	registry.registerParser(std::make_shared<LoggerConfigParser>());
	        	registry.registerParser(std::make_shared<DefaultLoadingConfigParser>());
	        	registry.registerParser(std::make_shared<VsyncConfigParser>());
	        	registry.registerParser(std::make_shared<LanguageConfigParser>());
	        	registry.registerParser(std::make_shared<CachingConfigParser>());
	        	registry.registerParser(std::make_shared<AppCachingConfigParser>());
	        	return registry;
	        }

        	inline appsettings::AppSettings loadAppSettingsFromStream(core::stream::InStreamInterface& stream)
	        {
	        	const static NodeParserRegistry app_settings_registry = createAppSettingsRegistry();

	        	if (!stream.is_open())
	        	{
	        		throw exception::parserexception::ParserException("Stream Error", "Input stream is not open.");
	        	}

	        	int64_t size = core::stream::utils::instream_size(stream);
	        	if (size <= 0)
	        	{
	        		throw exception::parserexception::ParserException("Stream Error", "Input stream is empty or size is invalid.");
	        	}

	        	std::vector<char> buffer(size);
	        	size_t bytes_read = stream.read(buffer.data(), size);

	        	if (bytes_read != static_cast<size_t>(size))
	        	{
	        		throw exception::parserexception::ParserException("Stream Error", "Failed to read the entire stream.");
	        	}

	        	YAML::Node root_node = YAML::Load(std::string(buffer.begin(), buffer.end()));

	        	const YAML::Node& app_yaml_node = root_node["app"];
	        	if (!app_yaml_node)
	        	{
	        		throw exception::parserexception::ParserException("Format Error", "YAML must contain a root 'app' node.");
	        	}

	        	auto parsed_data = app_settings_registry.parseNodeByKeyOrType("app", app_yaml_node);

	        	if (const auto* settings = parsed_data->getAs<appsettings::AppSettings>())
	        	{
	        		return *settings;
	        	}

	        	throw exception::parserexception::ParserException("Internal Parser Error", "Parsed data could not be cast to AppSettings.");
	        }

        	inline NodeParserRegistry createThemeConfigRegistry()
	        {
	        	using namespace theme;
	        	NodeParserRegistry registry;
	        	registry.registerParser(std::make_shared<AreaScaleParser>());
	        	registry.registerParser(std::make_shared<AnimationParser>());
	        	registry.registerParser(std::make_shared<ThemeResourceParser>());
	        	registry.registerParser(std::make_shared<ThemeParser>());
	        	return registry;
	        }

        	inline theme::ThemeConfig loadThemeConfigFromStream(core::stream::InStreamInterface& stream)
	        {
	        	stream.seek(0, core::stream::SeekMode::Begin);

	        	const static NodeParserRegistry theme_registry = createThemeConfigRegistry();

	        	if (!stream.is_open())
	        	{
	        		throw exception::parserexception::ParserException("Stream Error", "Input stream is not open.");
	        	}

	        	std::vector<char> buffer(core::stream::utils::instream_size(stream));
	        	stream.read(buffer.data(), buffer.size());
	        	std::string content(buffer.begin(), buffer.end());
	        	YAML::Node root_node = YAML::Load(content);

	        	const auto& theme_node = util::getYamlNodeElseThrow(root_node, "theme_config", "ThemeLoader");

	        	auto parsed_data = theme_registry.parseNodeByKeyOrType("theme_config", theme_node);

	        	if (const auto* settings = parsed_data->getAs<theme::ThemeConfig>())
	        	{
	        		return *settings;
	        	}

	        	throw exception::parserexception::ParserException("Internal Loader Error", "Could not cast parsed data to ThemeConfig.");
	        }

        	inline NodeParserRegistry createThemeGeneratorConfigRegistry()
	        {
	        	using namespace theme;
	        	NodeParserRegistry registry;
	        	registry.registerParser(std::make_shared<ThemeGeneratorParser>());
	        	return registry;
	        }

        	inline theme::ThemeGeneratorConfig loadThemeGeneratorConfigFromStream(core::stream::InStreamInterface& stream)
	        {
	        	stream.seek(0, core::stream::SeekMode::Begin);

	        	const static NodeParserRegistry theme_registry = createThemeGeneratorConfigRegistry();

	        	if (!stream.is_open())
	        	{
	        		throw exception::parserexception::ParserException("Stream Error", "Input stream is not open.");
	        	}

	        	std::vector<char> buffer(core::stream::utils::instream_size(stream));
	        	stream.read(buffer.data(), buffer.size());
	        	std::string content(buffer.begin(), buffer.end());
	        	YAML::Node root_node = YAML::Load(content);

	        	const auto& theme_node = util::getYamlNodeElseThrow(root_node, "theme_config", "ThemeLoader");

	        	auto parsed_data = theme_registry.parseNodeByKeyOrType("theme_generator_config", theme_node);

	        	if (const auto* settings = parsed_data->getAs<theme::ThemeGeneratorConfig>())
	        	{
	        		return *settings;
	        	}

	        	throw exception::parserexception::ParserException("Internal Loader Error", "Could not cast parsed data to ThemeConfig.");
	        }


        	inline std::shared_ptr<NodeParserRegistry> createGameDataParserRegistry()
	        {
	        	auto component_registry = createComponentParserRegistry();

	        	auto main_registry = std::make_shared<NodeParserRegistry>();

	        	main_registry->registerParser(std::make_shared<ResourceDeclarationParser>());
	        	main_registry->registerParser(std::make_shared<DataDeclarationParser>());
	        	main_registry->registerParser(std::make_shared<DeclarationsParser>());
	        	main_registry->registerParser(std::make_shared<DefinitionParser>());
	        	main_registry->registerParser(std::make_shared<DefinitionsParser>());

	        	main_registry->registerParser(std::make_shared<EntityParser>(component_registry));

	        	main_registry->registerParser(std::make_shared<GameDataParser>());

	        	return main_registry;
	        }

        }
    }
}
