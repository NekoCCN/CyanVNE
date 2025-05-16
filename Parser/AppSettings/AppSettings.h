#pragma once
#include <yaml-cpp/yaml.h>
#include <Parser/ParserYamlInterface/ParserYamlInterface.h>
#include <Parser/ParserException/ParserException.h>
#include <Parser/ParserUtil/ParserUtil.h>
#include <Core/Stream/Stream.h>
#include <string>
#include <vector>
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
				std::string title;

				bool is_default_loading;
				std::string default_loading_file;

				bool is_ratio_window;
				int window_width;
				int window_height;

				bool is_fullscreen;
				bool is_windowed_fullscreen;

				bool is_vsync;
				int fps;

				std::string graphics_driver;

				std::string theme_pack_path;

				std::set<std::string> advantage_languages_support;
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

                AppSettings get()
				{
					return settings_;
				}
			};
		}
	}
}