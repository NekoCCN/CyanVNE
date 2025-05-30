#pragma once
#include <yaml-cpp/yaml.h>
#include <Parser/ParserYamlInterface/ParserYamlInterface.h>
#include <Core/Stream/Stream.h>
#include <Core/Logger/Logger.h>
#include <string>
#include <set>
#include <memory>

namespace cyanvne
{
    namespace parser
    {
        namespace appsettings
        {
            struct AppSettings
            {
                std::string title = "Cyanvne";

                bool is_default_loading = false;
                std::string default_loading_file;

                bool is_ratio_window = false;
                int window_width = 1024;
                int window_height = 612;

                bool is_fullscreen = false;
                bool is_windowed_fullscreen = true;

                bool is_vsync = true;
                int fps = -1;

                std::string graphics_driver;

                std::string theme_pack_path = "BasicTheme.cyanbin";

                bool enable_extra_languages_support = false;
                std::set<std::string> extra_languages_support;

                core::GlobalLogger::LoggerConfig logger_config_;

                bool is_default_font = false;
            };

            class AppSettingsParser : public ParserYamlInterface
            {
            private:
                inline static const std::string parser_name_ = "AppSettings config";

                AppSettings settings_;

                void parseAppNode(const YAML::Node& node);
            public:
                AppSettingsParser() = default;

                void parse(const std::shared_ptr<core::stream::InStreamInterface>& yaml_stream) override;
                
                AppSettings get() const
                {
                    return settings_;
                }
            };
        }
    }
}