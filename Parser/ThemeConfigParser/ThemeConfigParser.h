#pragma once
#include "Parser/Framework/Framework.h"

namespace cyanvne
{
	namespace parser
	{
		namespace theme
		{
			class AreaScaleParser : public INodeParser
			{
			public:
				std::unique_ptr<ParsedNodeData> parse(const YAML::Node& node, const NodeParserRegistry& registry) const override;
				std::string getParsableNodeType() const override;
			};

			class AnimationParser : public INodeParser
			{
			public:
				std::unique_ptr<ParsedNodeData> parse(const YAML::Node& node, const NodeParserRegistry& registry) const override;
				std::string getParsableNodeType() const override;
			};

			class ThemeResourceParser : public INodeParser
			{
			public:
				std::unique_ptr<ParsedNodeData> parse(const YAML::Node& node, const NodeParserRegistry& registry) const override;
				std::string getParsableNodeType() const override;
			};

			class ThemeParser : public INodeParser
			{
			private:
				static constexpr std::vector<std::string> REQUIRED_KEY_ = {
					"main_menu_icon",
					"background",
					"create_cyan_data",
					"close",
					"save_process",
					"load_process",
					"history",
					"setting_in_game",
					"hide_dialog"
				};
			public:
				std::unique_ptr<ParsedNodeData> parse(const YAML::Node& node, const NodeParserRegistry& registry) const override;
				std::string getParsableNodeType() const override;
			};

			class ThemeGeneratorParser : public INodeParser
			{
			private:
				static constexpr std::vector<std::string> REQUIRED_KEY_ = {
					"main_menu_icon",
					"background",
					"create_cyan_data",
					"close",
					"save_process",
					"load_process",
					"history",
					"setting_in_game",
					"hide_dialog"
				};
			public:
				std::unique_ptr<ParsedNodeData> parse(const YAML::Node& node, const NodeParserRegistry& registry) const override;
				std::string getParsableNodeType() const override;
			};
		}
	}
}
