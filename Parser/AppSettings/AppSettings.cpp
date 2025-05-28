#include "AppSettings.h"
#include <Parser/ParserUtil/ParserUtil.h>
#include <Parser/ParserException/ParserException.h>
#include <format>

namespace cyanvne
{
    namespace parser
    {
        namespace appsettings
        {

            void AppSettingsParser::parseAppNode(const YAML::Node& node)
            {
                settings_.title = util::getScalarNodeElseThrow<std::string>(node, "title", parser_name_);

                const YAML::Node& default_loading_node = util::getYamlNodeElseThrow(node, "default_loading", parser_name_);
                settings_.is_default_loading = util::getScalarNodeElseThrow<bool>(default_loading_node, "enable", parser_name_);
                if (settings_.is_default_loading)
                {
                    settings_.default_loading_file = util::getScalarNodeElseThrow<std::string>(default_loading_node, "default_loading_file", parser_name_);
                }
                else
                {
                    settings_.default_loading_file.clear();
                }

                const YAML::Node& ratio_window_node = util::getYamlNodeElseThrow(node, "ratio_window", parser_name_);
                settings_.is_ratio_window = util::getScalarNodeElseThrow<bool>(ratio_window_node, "enable", parser_name_);
                settings_.window_height = util::getScalarNodeElseThrow<int>(ratio_window_node, "window_height", parser_name_);
                settings_.window_width = util::getScalarNodeElseThrow<int>(ratio_window_node, "window_width", parser_name_);

                settings_.is_fullscreen = util::getScalarNodeElseThrow<bool>(node, "fullscreen_mode", parser_name_);
                settings_.is_windowed_fullscreen = util::getScalarNodeElseThrow<bool>(node, "windowed_fullscreen_mode", parser_name_);

                const YAML::Node& vsync_node = util::getYamlNodeElseThrow(node, "vsync", parser_name_);
                settings_.is_vsync = util::getScalarNodeElseThrow<bool>(vsync_node, "enable", parser_name_);
                if (settings_.is_vsync)
                {
                    settings_.fps = util::getScalarNodeElseThrow<int>(vsync_node, "fps", parser_name_);
                }
                else
                {
                    settings_.fps = 0;
                }

                try
                {
                    settings_.graphics_driver = util::getScalarNodeElseThrow<std::string>(node, "graphics_context", parser_name_);
                }
                catch (const exception::parserexception::ParserException&)
                {
                    settings_.graphics_driver = "";
                }

                settings_.theme_pack_path = util::getScalarNodeElseThrow<std::string>(node, "theme_pack_path", parser_name_);

                const YAML::Node& lang_support_node = util::getYamlNodeElseThrow(node, "extra_languages_support", parser_name_);
                settings_.enable_extra_languages_support = util::getScalarNodeElseThrow<bool>(lang_support_node, "enable", parser_name_);
                if (settings_.enable_extra_languages_support)
                {
                    std::vector<std::string> lang_vector = util::getSequenceAsVectorElseThrow<std::string>(lang_support_node, "languages", parser_name_);
                    settings_.extra_languages_support.insert(lang_vector.begin(), lang_vector.end());
                }
                else
                {
                    settings_.extra_languages_support.clear();
                }

                auto resolveLogLevel = [](std::string& str) -> spdlog::level::level_enum
                    {
                        std::ranges::transform(str, str.begin(), tolower);
                        switch (str)
                        {
                        case "trace":
                            return spdlog::level::trace;
                        case "debug":
                            return spdlog::level::debug;
                        case "info":
                            return spdlog::level::info;
                        case "warn":
                            return spdlog::level::warn;
                        case "error":
                            return spdlog::level::err;
                        case "critical":
                            return spdlog::level::critical;
                        case "off":
                            return spdlog::level::off;
                        default:
                            throw exception::parserexception::ParserException(parser_name_ + " file(.yml) format error",
                                std::format("{:s} log level is not supported.", str));
                        }
                    };
                const YAML::Node& log_node = util::getYamlNodeElseThrow(node, "log", parser_name_);
                std::string console_log_level_str = util::getScalarNodeElseThrow<std::string>(log_node, "console_log_level", parser_name_);
                settings_.logger_config_.console_log_level = resolveLogLevel(console_log_level_str);
                const YAML::Node& file_log_node = util::getYamlNodeElseThrow(log_node, "file_log_level", parser_name_);
                settings_.logger_config_.enable_file = util::getScalarNodeElseThrow<bool>(file_log_node, "enable", parser_name_);
                if (settings_.logger_config_.enable_file)
                {
                    settings_.logger_config_.is_timestamped = util::getScalarNodeElseThrow<bool>(file_log_node, "timestamped", parser_name_);
                    std::string file_log_level_str = util::getScalarNodeElseThrow<std::string>(file_log_node, "file_log_level", parser_name_);
                    settings_.logger_config_.file_log_level = resolveLogLevel(file_log_level_str);
                    settings_.logger_config_.file_path = util::getScalarNodeElseThrow<std::string>(file_log_node, "file_path", parser_name_);
                    settings_.logger_config_.is_rotating = util::getScalarNodeElseThrow<bool>(file_log_node, "rotating", parser_name_);
                }
            }

            void AppSettingsParser::parse(
                const std::shared_ptr<core::stream::InStreamInterface>& yaml_stream)
            {
                if (!yaml_stream)
                {
                    throw exception::parserexception::ParserException(parser_name_ + " error", "Input stream is null.");
                }

                yaml_stream->seek(0, core::stream::SeekMode::End);
                size_t size = yaml_stream->tell();

                if (size == 0)
                {
                    throw exception::parserexception::ParserException(parser_name_ + " file(.yml) format error",
                        parser_name_ + " file(.yml) is empty.");
                }

                std::string yaml_content;
                try
                {
                    yaml_content.resize(size);
                }
                catch (const std::bad_alloc&)
                {
                    throw exception::parserexception::ParserException(parser_name_ + " file(.yml) format error",
                        parser_name_ + " file(.yml) is too large to allocate buffer.");
                }

                yaml_stream->seek(0, core::stream::SeekMode::Begin);
                size_t read_size = yaml_stream->read(yaml_content.data(), size);

                if (read_size != size)
                {
                    throw exception::parserexception::ParserException(parser_name_ + " error", "Failed to read the entire stream content.");
                }

                YAML::Node root_node;
                try
                {
                    root_node = YAML::Load(yaml_content);
                }
                catch (const YAML::ParserException& e)
                {
                    throw exception::parserexception::ParserException(parser_name_ + " file(.yml) syntax error",
                        std::format("Error parsing YAML: {:s} at line {:d}, column {:d}",
                            e.msg, e.mark.line + 1, e.mark.column + 1));
                }
                catch (const YAML::Exception& e)
                {
                    throw exception::parserexception::ParserException(parser_name_ + " file(.yml) error",
                        std::format("YAML library error: {:s}", e.what()));
                }

                if (!root_node.IsDefined() || root_node.IsNull())
                {
                    throw exception::parserexception::ParserException(parser_name_ + " file(.yml) format error",
                        parser_name_ + " file(.yml) contains no valid document or is empty.");
                }

                const YAML::Node& app_node_yaml = util::getYamlNodeElseThrow(root_node, "app", parser_name_);

                parseAppNode(app_node_yaml);
            }

        }
    }
}