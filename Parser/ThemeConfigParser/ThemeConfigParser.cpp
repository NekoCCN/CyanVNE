#include "ThemeConfigParser.h"
#include <Parser/ParserException/ParserException.h>
#include <format>

namespace cyanvne
{
    namespace parser
    {
        namespace theme
        {
            std::vector<double> parseAreaScaleNode(const YAML::Node& parent_node, const std::string& key, const std::string& parser_name)
            {
                const YAML::Node& scale_node = util::getYamlNodeElseThrow(parent_node, key, parser_name);
                if (!scale_node.IsMap())
                {
                    throw exception::parserexception::ParserException(
                        parser_name + " format error",
                        std::format("Node '{}' under '{}' is not a map for area_scale.", key, parser_name));
                }
                std::vector<double> scale_values;
                scale_values.push_back(util::getScalarNodeElseThrow<double>(scale_node, "x", parser_name));
                scale_values.push_back(util::getScalarNodeElseThrow<double>(scale_node, "y", parser_name));
                scale_values.push_back(util::getScalarNodeElseThrow<double>(scale_node, "w", parser_name));
                scale_values.push_back(util::getScalarNodeElseThrow<double>(scale_node, "h", parser_name));
                return scale_values;
            }

            std::vector<uint32_t> parseSpriteNode(const YAML::Node& sprite_yaml_node, const std::string& parser_name, const std::string& resource_key, int sprite_index)
            {
                if (!sprite_yaml_node.IsMap())
                {
                    throw exception::parserexception::ParserException(
                        parser_name + " format error",
                        std::format("Sprite at index {} for resource '{}' is not a map.", sprite_index, resource_key));
                }
                std::vector<uint32_t> sprite_values;
                sprite_values.push_back(util::getScalarNodeElseThrow<uint32_t>(sprite_yaml_node, "x", parser_name));
                sprite_values.push_back(util::getScalarNodeElseThrow<uint32_t>(sprite_yaml_node, "y", parser_name));
                sprite_values.push_back(util::getScalarNodeElseThrow<uint32_t>(sprite_yaml_node, "w", parser_name));
                sprite_values.push_back(util::getScalarNodeElseThrow<uint32_t>(sprite_yaml_node, "h", parser_name));
                return sprite_values;
            }

            void ThemeConfigParser::parseResourceNodeForThemeConfig(
                const YAML::Node& resource_yaml_node,
                const std::string& resource_key,
                bool& enable_flag,
                std::string& key_field,
                int32_t& ms_per_frame,
                std::vector<std::vector<uint32_t>>& sprite_sheet,
                std::vector<double>& area_scale_vector)
            {
                enable_flag = false;
                key_field = resource_key;
                ms_per_frame = -1;
                sprite_sheet.clear();
                area_scale_vector.clear();

                if (resource_yaml_node.IsScalar() && resource_yaml_node.as<std::string>() == "false")
                {
                    return;
                }
                if (!resource_yaml_node.IsMap())
                {
                    throw exception::parserexception::ParserException(
                        parser_name_ + " format error",
                        std::format("Resource node '{}' is not a map or 'false'.", resource_key));
                }

                enable_flag = true;

                if (resource_yaml_node["area_scale"])
                {
                    area_scale_vector = parseAreaScaleNode(resource_yaml_node, "area_scale", parser_name_);
                }

                if (resource_yaml_node["animation"])
                {
                    const YAML::Node& animation_node = resource_yaml_node["animation"];
                    if (animation_node.IsScalar() && animation_node.as<std::string>() == "false")
                    {
                        ms_per_frame = -1;
                        sprite_sheet.clear();
                    }
                    else if (animation_node.IsMap())
                    {
                        ms_per_frame = util::getScalarNodeElseThrow<int32_t>(animation_node, "ms_per_frame", parser_name_);

                        const YAML::Node& sheet_node_yaml = util::getYamlNodeElseThrow(animation_node, "sprite_sheet", parser_name_);
                        if (!sheet_node_yaml.IsSequence())
                        {
                            throw exception::parserexception::ParserException(
                                parser_name_ + " format error",
                                std::format("Node 'sprite_sheet' under animation for '{}' is not a sequence.", resource_key));
                        }
                        sprite_sheet.reserve(sheet_node_yaml.size());
                        for (std::size_t i = 0; i < sheet_node_yaml.size(); ++i)
                        {
                            sprite_sheet.push_back(parseSpriteNode(sheet_node_yaml[i], parser_name_, resource_key, static_cast<int>(i)));
                        }
                    }
                    else
                    {
                        throw exception::parserexception::ParserException(
                            parser_name_ + " format error",
                            std::format("Node 'animation' for '{}' must be a map or 'false'.", resource_key));
                    }
                }
            }

            void ThemeConfigParser::parse(const std::shared_ptr<core::stream::InStreamInterface>& yaml_stream)
            {
                if (!yaml_stream)
                {
                    throw exception::parserexception::ParserException(parser_name_ + " error", "Input stream is null.");
                }

                yaml_stream->seek(0, core::stream::SeekMode::End);
                size_t size = yaml_stream->tell();

                if (size == 0)
                {
                    throw exception::parserexception::ParserException(parser_name_ + " file format error", parser_name_ + " file is empty.");
                }
                std::string yaml_content;
                try
                {
                    yaml_content.resize(size);
                }
                catch (const std::bad_alloc&)
                {
                    throw exception::parserexception::ParserException(parser_name_ + " file format error", parser_name_ + " file is too large.");
                }
                yaml_stream->seek(0, core::stream::SeekMode::Begin);
                size_t read_size = yaml_stream->read(yaml_content.data(), size);
                if (read_size != size)
                {
                    throw exception::parserexception::ParserException(parser_name_ + " error", "Failed to read the entire stream.");
                }

                YAML::Node root_node;
                try
                {
                    root_node = YAML::Load(yaml_content);
                }
                catch (const YAML::ParserException& e)
                {
                    throw exception::parserexception::ParserException(parser_name_ + " YAML syntax error",
                        std::format("Error parsing YAML: {} at line {}, column {}", e.msg, e.mark.line + 1, e.mark.column + 1));
                }
                catch (const YAML::Exception& e)
                {
                    throw exception::parserexception::ParserException(parser_name_ + " YAML library error", std::format("YAML error: {}", e.what()));
                }

                if (!root_node.IsDefined() || root_node.IsNull() || !root_node.IsMap())
                {
                    throw exception::parserexception::ParserException(parser_name_ + " format error", "Root node is not a valid map or is empty.");
                }
                const YAML::Node& theme_node = root_node;

                config_.name = util::getScalarNodeElseThrow<std::string>(theme_node, "name", parser_name_);

                const YAML::Node& font_node = util::getYamlNodeElseThrow(theme_node, "built_in_font", parser_name_);
                if (font_node.IsScalar() && font_node.as<std::string>() == "false")
                {
                    config_.enable_built_in_font = false;
                    config_.basic_font_key.clear();
                }
                else {
                    config_.enable_built_in_font = true;
                    config_.basic_font_key = "built_in_font";
                }

                auto parse_tc_resource = [&](const std::string& key,
                    bool& enable, std::string& res_key, int32_t& ms,
                    std::vector<std::vector<uint32_t>>& sheet, std::vector<double>& scale) 
                    {
                        if (theme_node[key]) {
                            parseResourceNodeForThemeConfig(theme_node[key], key, enable, res_key, ms, sheet, scale);
                        }
                        else
                        {
                            enable = false;
                            res_key = key;
                            ms = -1;
                            sheet.clear();
                            scale.clear();
                        }
                    };

                parse_tc_resource("main_menu_icon", config_.enable_main_menu_icon, config_.main_menu_icon_key, config_.main_menu_icon_ms_per_frame, config_.main_menu_icon_sprite_sheet, config_.main_menu_icon_rendering_area_scale);
                parse_tc_resource("background", config_.enable_background, config_.background_key, config_.background_ms_per_frame, config_.background_sprite_sheet, config_.background_rendering_area_scale);
                parse_tc_resource("create_cyan_data", config_.enable_create_cyan_data, config_.create_cyan_data_key, config_.create_cyan_data_ms_per_frame, config_.create_cyan_data_sprite_sheet, config_.create_cyan_data_rendering_area_scale);
                parse_tc_resource("create_cyan_theme", config_.enable_create_cyan_theme, config_.create_cyan_theme_key, config_.create_cyan_theme_ms_per_frame, config_.create_cyan_theme_sprite_sheet, config_.create_cyan_theme_rendering_area_scale);
                parse_tc_resource("load_cyan_data", config_.enable_load_cyan_data, config_.load_cyan_data_key, config_.load_cyan_data_ms_per_frame, config_.load_cyan_data_sprite_sheet, config_.load_cyan_data_rendering_area_scale);
                parse_tc_resource("settings", config_.enable_settings, config_.settings_key, config_.settings_ms_per_frame, config_.settings_sprite_sheet, config_.settings_rendering_area_scale);
                parse_tc_resource("close", config_.enable_close, config_.close_key, config_.close_ms_per_frame, config_.close_sprite_sheet, config_.close_rendering_area_scale);
                parse_tc_resource("dialog_image", config_.enable_dialog_image, config_.dialog_image_key, config_.dialog_image_ms_per_frame, config_.dialog_image_sprite_sheet, config_.dialog_image_rendering_area_scale);
                parse_tc_resource("save_process", config_.enable_save_process, config_.save_process_key, config_.save_process_ms_per_frame, config_.save_process_sprite_sheet, config_.save_process_rendering_area_scale);
                parse_tc_resource("load_process", config_.enable_load_process, config_.load_process_key, config_.load_process_ms_per_frame, config_.load_process_sprite_sheet, config_.load_process_rendering_area_scale);
                parse_tc_resource("history", config_.enable_history, config_.history_key, config_.history_ms_per_frame, config_.history_sprite_sheet, config_.history_rendering_area_scale);
                parse_tc_resource("setting_in_game", config_.enable_setting_in_game, config_.setting_in_game_key, config_.setting_in_game_ms_per_frame, config_.setting_in_game_sprite_sheet, config_.setting_in_game_rendering_area_scale);
                parse_tc_resource("hide_dialog", config_.enable_hide_dialog, config_.hide_dialog_key, config_.hide_dialog_ms_per_frame, config_.hide_dialog_sprite_sheet, config_.hide_dialog_rendering_area_scale);
            }

            void ThemeGeneratorConfigParser::parseResourceNodeForGeneratorConfig(
                const YAML::Node& resource_yaml_node,
                const std::string& resource_key,
                bool& enable_flag,
                std::string& key_field,
                std::string& path_field)
            {
                enable_flag = false;
                key_field = resource_key;
                path_field.clear();

                if (resource_yaml_node.IsScalar() && resource_yaml_node.as<std::string>() == "false")
                {
                    return;
                }
                if (!resource_yaml_node.IsMap())
                {
                    throw exception::parserexception::ParserException(
                        parser_name_ + " format error",
                        std::format("Resource node '{}' for generator is not a map or 'false'.", resource_key));
                }

                enable_flag = true;
                path_field = util::getScalarNodeElseThrow<std::string>(resource_yaml_node, "path", parser_name_);
            }

            void ThemeGeneratorConfigParser::parse(const std::shared_ptr<core::stream::InStreamInterface>& yaml_stream)
            {
                if (!yaml_stream)
                {
                    throw exception::parserexception::ParserException(parser_name_ + " error", "Input stream is null.");
                }
                yaml_stream->seek(0, core::stream::SeekMode::End);
                size_t size = yaml_stream->tell();

                if (size == 0)
                {
                    throw exception::parserexception::ParserException(parser_name_ + " file format error", parser_name_ + " file is empty.");
                }
                std::string yaml_content;
                try
                {
                    yaml_content.resize(size);
                }
                catch (const std::bad_alloc&)
                {
                    throw exception::parserexception::ParserException(parser_name_ + " file format error", parser_name_ + " file is too large.");
                }
                yaml_stream->seek(0, core::stream::SeekMode::Begin);
                size_t read_size = yaml_stream->read(yaml_content.data(), size);
                if (read_size != size)
                {
                    throw exception::parserexception::ParserException(parser_name_ + " error", "Failed to read the entire stream.");
                }

                YAML::Node root_node;
                try
                {
                    root_node = YAML::Load(yaml_content);
                }
                catch (const YAML::ParserException& e)
                {
                    throw exception::parserexception::ParserException(parser_name_ + " YAML syntax error",
                        std::format("Error parsing YAML: {} at line {}, column {}", e.msg, e.mark.line + 1, e.mark.column + 1));
                }
                catch (const YAML::Exception& e)
                {
                    throw exception::parserexception::ParserException(parser_name_ + " YAML library error", std::format("YAML error: {}", e.what()));
                }

                const YAML::Node& generator_node = util::getYamlNodeElseThrow(root_node, "theme_generator_config", parser_name_);

                auto parse_tgc_resource = [&](const std::string& key,
                    bool& enable, std::string& res_key, std::string& path)
                    {
                        if (generator_node[key])
                        {
                            parseResourceNodeForGeneratorConfig(generator_node[key], key, enable, res_key, path);
                        }
                        else
                        {
                            enable = false;
                            res_key = key;
                            path.clear();
                        }
                    };

                parse_tgc_resource("built_in_font", config_.enable_built_in_font, config_.basic_built_in_font_key, config_.basic_built_in_font_path);
                parse_tgc_resource("main_menu_icon", config_.enable_main_menu_logo, config_.main_menu_logo_key, config_.main_menu_logo_path);
                parse_tgc_resource("background", config_.enable_background, config_.background_key, config_.background_path);
                parse_tgc_resource("create_cyan_data", config_.enable_create_cyan_data, config_.create_cyan_data_key, config_.create_cyan_data_path);
                parse_tgc_resource("create_cyan_theme", config_.enable_create_cyan_theme, config_.create_cyan_theme_key, config_.create_cyan_theme_path);
                parse_tgc_resource("load_cyan_data", config_.enable_load_cyan_data, config_.load_cyan_data_key, config_.load_cyan_data_path);
                parse_tgc_resource("settings", config_.enable_settings, config_.settings_key, config_.settings_path);
                parse_tgc_resource("close", config_.enable_close, config_.close_key, config_.close_path);
                parse_tgc_resource("dialog_image", config_.enable_dialog_image, config_.dialog_image_key, config_.dialog_image_path);
                parse_tgc_resource("save_process", config_.enable_save_process, config_.save_process_key, config_.save_process_path);
                parse_tgc_resource("load_process", config_.enable_load_process, config_.load_process_key, config_.load_process_path);
                parse_tgc_resource("history", config_.enable_history, config_.history_key, config_.history_path);
                parse_tgc_resource("setting_in_game", config_.enable_setting_in_game, config_.setting_in_game_key, config_.setting_in_game_path);
                parse_tgc_resource("hide_dialog", config_.enable_hide_dialog, config_.hide_dialog_key, config_.hide_dialog_path);
            }
        }
    }
}