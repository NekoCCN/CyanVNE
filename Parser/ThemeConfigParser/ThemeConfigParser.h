#pragma once
#include <yaml-cpp/yaml.h>
#include <Parser/ParserYamlInterface/ParserYamlInterface.h>
#include <Parser/ParserUtil/ParserUtil.h>
#include <Parser/ThemeConfig/ThemeConfig.h>
#include <Core/Stream/Stream.h>
#include <string>
#include <memory>
#include <vector>

namespace cyanvne
{
    namespace parser
    {
        struct ThemeConfig;
        struct ThemeGeneratorConfig;

        namespace theme
        {
            std::vector<double> parseAreaScaleNode(const YAML::Node& parent_node, const std::string& key, const std::string& parser_name);
            std::vector<uint32_t> parseSpriteNode(const YAML::Node& sprite_yaml_node, const std::string& parser_name, const std::string& resource_key, int sprite_index);

            class ThemeConfigParser : public ParserYamlInterface
            {
            private:
                inline static const std::string parser_name_ = "ThemeConfig (from YAML)";
                ThemeConfig config_;

                static void parseResourceNodeForThemeConfig(
                    const YAML::Node& resource_yaml_node,
                    const std::string& resource_key,
                    bool& enable_flag,
                    std::string& key_field,
                    int32_t& ms_per_frame,
                    std::vector<std::vector<uint32_t>>& sprite_sheet,
                    std::vector<double>& area_scale_vector);

            public:
                ThemeConfigParser() = default;
                void parse(const std::shared_ptr<core::stream::InStreamInterface>& yaml_stream) override;
                ThemeConfig& get()
                {
                    return config_;
                }
            };

            class ThemeGeneratorConfigParser : public ParserYamlInterface
            {
            private:
                inline static const std::string parser_name_ = "ThemeGeneratorConfig (from YAML)";
                ThemeGeneratorConfig config_;

                static void parseResourceNodeForGeneratorConfig(
                    const YAML::Node& resource_yaml_node,
                    const std::string& resource_key,
                    bool& enable_flag,
                    std::string& key_field,
                    std::string& path_field);

            public:
                ThemeGeneratorConfigParser() = default;
                void parse(const std::shared_ptr<core::stream::InStreamInterface>& yaml_stream) override;
                ThemeGeneratorConfig get() const
                {
                    return config_;
                }
            };
        }
    }
}