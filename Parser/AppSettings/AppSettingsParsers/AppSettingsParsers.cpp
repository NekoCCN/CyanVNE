#include "AppSettingsParsers.h"
#include <algorithm>

namespace cyanvne
{
    namespace parser
    {
        namespace appsettings
        {
            std::unique_ptr<ParsedNodeData> LoggerConfigParser::parse(const YAML::Node& node, const NodeParserRegistry& registry) const
            {
                core::GlobalLogger::LoggerConfig config;
                auto resolveLogLevel = [](std::string str) -> spdlog::level::level_enum
                    {
                        std::ranges::transform(str, str.begin(), ::tolower);

                        if (str == "trace") return spdlog::level::trace;
                        if (str == "debug") return spdlog::level::debug;
                        if (str == "info") return spdlog::level::info;
                        if (str == "warn") return spdlog::level::warn;
                        if (str == "error") return spdlog::level::err;
                        if (str == "critical") return spdlog::level::critical;
                        if (str == "off") return spdlog::level::off;
                        throw exception::parserexception::ParserException("Log config format error",
                            std::format("'{}' is not a supported log level.", str));
                    };

                std::string console_level_str = util::getScalarNodeElseThrow<std::string>(node, "console_log_level", getParsableNodeType());
                config.console_log_level = resolveLogLevel(console_level_str);

                const YAML::Node& file_node = util::getYamlNodeElseThrow(node, "file", getParsableNodeType());
                config.enable_file = util::getScalarNodeElseThrow<bool>(file_node, "enable", getParsableNodeType());

                if (config.enable_file)
                {
                    config.is_timestamped = util::getScalarNodeElseThrow<bool>(file_node, "timestamped", getParsableNodeType());
                    std::string file_level_str = util::getScalarNodeElseThrow<std::string>(file_node, "file_log_level", getParsableNodeType());
                    config.file_log_level = resolveLogLevel(file_level_str);
                    config.file_path = util::getScalarNodeElseThrow<std::string>(file_node, "file_path", getParsableNodeType());
                    config.is_rotating = util::getScalarNodeElseThrow<bool>(file_node, "rotating", getParsableNodeType());
                }

                return std::make_unique<ParsedNodeData>(std::move(config), getParsableNodeType());
            }

            std::string LoggerConfigParser::getParsableNodeType() const
            {
                return "log";
            }

            std::unique_ptr<ParsedNodeData> DefaultLoadingConfigParser::parse(const YAML::Node& node, const NodeParserRegistry& registry) const
            {
                DefaultLoadingConfig config;
                config.is_enabled = util::getScalarNodeElseThrow<bool>(node, "enable", getParsableNodeType());
                if (config.is_enabled)
                {
                    config.file = util::getScalarNodeElseThrow<std::string>(node, "default_loading_file", getParsableNodeType());
                }
                return std::make_unique<ParsedNodeData>(std::move(config), getParsableNodeType());
            }

            std::string DefaultLoadingConfigParser::getParsableNodeType() const { return "default_loading"; }

            std::unique_ptr<ParsedNodeData> WindowConfigParser::parse(const YAML::Node& node, const NodeParserRegistry& registry) const
            {
                WindowConfig config;
                config.is_fullscreen = util::getScalarNodeElseThrow<bool>(node, "fullscreen_mode", getParsableNodeType());
                config.is_windowed_fullscreen = util::getScalarNodeElseThrow<bool>(node, "windowed_fullscreen_mode", getParsableNodeType());

                const auto& ratio_node = util::getYamlNodeElseThrow(node, "ratio_window", getParsableNodeType());
                config.is_ratio_window = util::getScalarNodeElseThrow<bool>(ratio_node, "enable", getParsableNodeType());
                config.width = util::getScalarNodeElseThrow<int>(ratio_node, "window_width", getParsableNodeType());
                config.height = util::getScalarNodeElseThrow<int>(ratio_node, "window_height", getParsableNodeType());

                return std::make_unique<ParsedNodeData>(config, getParsableNodeType());
            }

            std::string WindowConfigParser::getParsableNodeType() const { return "window"; }

            std::unique_ptr<ParsedNodeData> VsyncConfigParser::parse(const YAML::Node& node, const NodeParserRegistry& registry) const
            {
                VsyncConfig config;
                config.is_enabled = util::getScalarNodeElseThrow<bool>(node, "enable", getParsableNodeType());
                if (config.is_enabled)
                {
                    config.fps = util::getScalarNodeElseThrow<int>(node, "fps", getParsableNodeType());
                }
                else
                {
                    config.fps = 0;
                }
                return std::make_unique<ParsedNodeData>(config, getParsableNodeType());
            }

            std::string VsyncConfigParser::getParsableNodeType() const
            {
                return "vsync";
            }

            std::unique_ptr<ParsedNodeData> LanguageConfigParser::parse(const YAML::Node& node, const NodeParserRegistry& registry) const
            {
                LanguageConfig config;
                config.is_enabled = util::getScalarNodeElseThrow<bool>(node, "enable", getParsableNodeType());
                if (config.is_enabled)
                {
                    auto lang_vector = util::getSequenceAsVectorElseThrow<std::string>(node, "languages", getParsableNodeType());
                    config.supported_languages.insert(lang_vector.begin(), lang_vector.end());
                }
                return std::make_unique<ParsedNodeData>(std::move(config), getParsableNodeType());
            }

            std::string LanguageConfigParser::getParsableNodeType() const
            {
                return "extra_languages_support";
            }

            std::unique_ptr<ParsedNodeData> CachingConfigParser::parse(const YAML::Node& node,
	            const NodeParserRegistry& registry) const
            {
                CachingConfig config;

                config.max_volatile_size = util::getScalarNodeElseThrow<uint64_t>(node, "max_volatile_size", getParsableNodeType());
                config.max_persistent_size = util::getScalarNodeElseThrow<uint64_t>(node, "max_persistent_size", getParsableNodeType());
                config.max_single_persistent_size = util::getScalarNodeElseThrow<uint64_t>(node, "max_single_persistent_size", getParsableNodeType());
            
                return std::make_unique<ParsedNodeData>(config, getParsableNodeType());
            }

            std::string CachingConfigParser::getParsableNodeType() const
            {
                return "caching_config";
            }

            std::unique_ptr<ParsedNodeData> AppCachingConfigParser::parse(const YAML::Node& node,
	            const NodeParserRegistry& registry) const
            {
                AppCachingConfig settings;

                settings.theme_caching_config = registry.useParserPraseSubNodeAs<CachingConfig>(node,
                    "caching_config", "theme_caching_config", getParsableNodeType());

                settings.process_data_caching_config = registry.useParserPraseSubNodeAs<CachingConfig>(node,
                    "caching_config","process_data_caching_config", getParsableNodeType());
            
                return std::make_unique<ParsedNodeData>(settings, getParsableNodeType());
            }

            std::string AppCachingConfigParser::getParsableNodeType() const
            {
                return "caching";
            }

            std::unique_ptr<ParsedNodeData> AppNodeParser::parse(const YAML::Node& node, const NodeParserRegistry& registry) const
            {
                AppSettings settings;

                settings.title = util::getScalarNodeElseThrow<std::string>(node, "title", getParsableNodeType());
                settings.theme_pack_path = util::getScalarNodeElseThrow<std::string>(node, "theme_pack_path", getParsableNodeType());

                if (const auto& driver_node = node["graphics_context"])
                {
                    settings.graphics_driver = driver_node.as<std::string>();
                }

                settings.default_loading = registry.parseSubNodeAs<DefaultLoadingConfig>(node, "default_loading", getParsableNodeType());
                settings.vsync = registry.parseSubNodeAs<VsyncConfig>(node, "vsync", getParsableNodeType());
                settings.languages = registry.parseSubNodeAs<LanguageConfig>(node, "extra_languages_support", getParsableNodeType());
                settings.logger = registry.parseSubNodeAs<core::GlobalLogger::LoggerConfig>(node, "log", getParsableNodeType());

                auto parsed_window_data = registry.parseNodeByKeyOrType("window", node);
                if (const auto* window_config = parsed_window_data->getAs<WindowConfig>())
                {
                    settings.window = *window_config;
                }
                else
                {
                    throw exception::parserexception::ParserException("Internal Parser Error", "Failed to cast parsed 'window' data.");
                }

                    settings.caching = registry.parseSubNodeAs<AppCachingConfig>(node, "caching", getParsableNodeType());

                return std::make_unique<ParsedNodeData>(std::move(settings), getParsableNodeType());
            }

            std::string AppNodeParser::getParsableNodeType() const
            {
                return "app";
            }
        }
    }
}
