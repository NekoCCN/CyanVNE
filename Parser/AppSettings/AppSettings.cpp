#include "AppSettings.h"

void cyanvne::parser::appsettings::AppSettingsParser::parseAppNode(const YAML::Node& node)
{
	settings_.title = util::getScalarNodeElseThrow<std::string>(node, "title", parser_name_);

	YAML::Node default_loading_node = util::getYamlNodeElseThrow(node, "default_loading", parser_name_);
	settings_.is_default_loading = util::getScalarNodeElseThrow<bool>(default_loading_node, "enable", parser_name_);
	if (settings_.is_default_loading)
	{
        settings_.default_loading_file = util::getScalarNodeElseThrow<std::string>(default_loading_node, "file", parser_name_);
	}

	YAML::Node is_ratio_window_node = util::getYamlNodeElseThrow(node, "ratio_window", parser_name_);
    settings_.is_ratio_window = util::getScalarNodeElseThrow<bool>(is_ratio_window_node, "enable", parser_name_);
	settings_.window_height = util::getScalarNodeElseThrow<int>(is_ratio_window_node, "window_height", parser_name_);
	settings_.window_width = util::getScalarNodeElseThrow<int>(is_ratio_window_node, "window_width", parser_name_);

	settings_.is_fullscreen = util::getScalarNodeElseThrow<bool>(node, "fullscreen_mode", parser_name_);
	settings_.is_windowed_fullscreen = util::getScalarNodeElseThrow<bool>(node, "windowed_fullscreen_mode", parser_name_);

	YAML::Node vsync_node = util::getYamlNodeElseThrow(node, "vsync", parser_name_);
    settings_.is_vsync = util::getScalarNodeElseThrow<bool>(vsync_node, "enabled", parser_name_);
	settings_.fps = util::getScalarNodeElseThrow<int>(node, "fps", parser_name_);

	try {
		settings_.graphics_driver = util::getScalarNodeElseThrow<std::string>(node, "graphics_context", parser_name_);
	}
	catch (const exception::parserexception::ParserException&)
	{
		settings_.graphics_driver = "";
	}

	settings_.theme_pack_path = util::getScalarNodeElseThrow<std::string>(node, "theme_pack_path", parser_name_);
}

void cyanvne::parser::appsettings::AppSettingsParser::parse(
	const std::shared_ptr<core::stream::InStreamInterface>& yaml_stream)
{
	yaml_stream->seek(0, core::stream::SeekMode::End);
	size_t size = yaml_stream->tell();

	std::vector<char> buffer;

	try
	{
		buffer.resize(size);
	}
	catch (const std::bad_alloc&)
	{
		throw exception::parserexception::ParserException("AppSettings config file(.yml) format error",
		                                                  "AppSettings config file(.yml) is too large.");
	}

	yaml_stream->seek(0, core::stream::SeekMode::Begin);
	yaml_stream->read(buffer.data(), size);
	yaml_stream->seek(0, core::stream::SeekMode::Begin);

	std::vector<YAML::Node> node_list = YAML::LoadAll(buffer.data());

	if (node_list.size() > 1)
	{
		throw exception::parserexception::ParserException("AppSettings config file(.yml) format error",
		                                                  "AppSettings config file(.yml) contains more than one document.");
	}
	if (node_list.empty())
	{
		throw exception::parserexception::ParserException("AppSettings config file(.yml) format error",
		                                                  "AppSettings config file(.yml) contains no document.");
	}
	YAML::Node node = node_list[0];

	if (!node["app"])
	{
		throw exception::parserexception::ParserException("AppSettings config file(.yml) format error",
		                                                  "AppSettings config file(.yml) contains no app node.");
	}
                    
	parseAppNode(node["app"]);
}
