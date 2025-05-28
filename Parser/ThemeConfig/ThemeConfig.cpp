#include "ThemeConfig.h"
#include <tuple>
#include <cmath>

namespace cyanvne
{
    namespace parser
    {
        auto ThemeConfig::getMembersTuple()
        {
            return std::tie(
                name, enable_built_in_font, basic_font_key,
                enable_main_menu_icon, main_menu_icon_key, main_menu_icon_ms_per_frame, main_menu_icon_sprite_sheet, main_menu_icon_rendering_area_scale,
                enable_background, background_key, background_ms_per_frame, background_sprite_sheet, background_rendering_area_scale,
                enable_create_cyan_data, create_cyan_data_key, create_cyan_data_ms_per_frame, create_cyan_data_sprite_sheet, create_cyan_data_rendering_area_scale,
                enable_create_cyan_theme, create_cyan_theme_key, create_cyan_theme_ms_per_frame, create_cyan_theme_sprite_sheet, create_cyan_theme_rendering_area_scale,
                enable_load_cyan_data, load_cyan_data_key, load_cyan_data_ms_per_frame, load_cyan_data_sprite_sheet, load_cyan_data_rendering_area_scale,
                enable_settings, settings_key, settings_ms_per_frame, settings_sprite_sheet, settings_rendering_area_scale,
                enable_close, close_key, close_ms_per_frame, close_sprite_sheet, close_rendering_area_scale,
                enable_dialog_image, dialog_image_key, dialog_image_ms_per_frame, dialog_image_sprite_sheet, dialog_image_rendering_area_scale,
                enable_save_process, save_process_key, save_process_ms_per_frame, save_process_sprite_sheet, save_process_rendering_area_scale,
                enable_load_process, load_process_key, load_process_ms_per_frame, load_process_sprite_sheet, load_process_rendering_area_scale,
                enable_history, history_key, history_ms_per_frame, history_sprite_sheet, history_rendering_area_scale,
                enable_setting_in_game, setting_in_game_key, setting_in_game_ms_per_frame, setting_in_game_sprite_sheet, setting_in_game_rendering_area_scale,
                enable_hide_dialog, hide_dialog_key, hide_dialog_ms_per_frame, hide_dialog_sprite_sheet, hide_dialog_rendering_area_scale
            );
        }

        auto ThemeConfig::getMembersTupleConst() const
        {
            return std::tie(
                name, enable_built_in_font, basic_font_key,
                enable_main_menu_icon, main_menu_icon_key, main_menu_icon_ms_per_frame, main_menu_icon_sprite_sheet, main_menu_icon_rendering_area_scale,
                enable_background, background_key, background_ms_per_frame, background_sprite_sheet, background_rendering_area_scale,
                enable_create_cyan_data, create_cyan_data_key, create_cyan_data_ms_per_frame, create_cyan_data_sprite_sheet, create_cyan_data_rendering_area_scale,
                enable_create_cyan_theme, create_cyan_theme_key, create_cyan_theme_ms_per_frame, create_cyan_theme_sprite_sheet, create_cyan_theme_rendering_area_scale,
                enable_load_cyan_data, load_cyan_data_key, load_cyan_data_ms_per_frame, load_cyan_data_sprite_sheet, load_cyan_data_rendering_area_scale,
                enable_settings, settings_key, settings_ms_per_frame, settings_sprite_sheet, settings_rendering_area_scale,
                enable_close, close_key, close_ms_per_frame, close_sprite_sheet, close_rendering_area_scale,
                enable_dialog_image, dialog_image_key, dialog_image_ms_per_frame, dialog_image_sprite_sheet, dialog_image_rendering_area_scale,
                enable_save_process, save_process_key, save_process_ms_per_frame, save_process_sprite_sheet, save_process_rendering_area_scale,
                enable_load_process, load_process_key, load_process_ms_per_frame, load_process_sprite_sheet, load_process_rendering_area_scale,
                enable_history, history_key, history_ms_per_frame, history_sprite_sheet, history_rendering_area_scale,
                enable_setting_in_game, setting_in_game_key, setting_in_game_ms_per_frame, setting_in_game_sprite_sheet, setting_in_game_rendering_area_scale,
                enable_hide_dialog, hide_dialog_key, hide_dialog_ms_per_frame, hide_dialog_sprite_sheet, hide_dialog_rendering_area_scale
            );
        }

        bool ThemeConfig::serialize(cyanvne::core::stream::OutStreamInterface& out) const
        {
            if (!out.is_open())
            {
                return false;
            }

            ThemeConfigFileHeader header_to_write;
            if (!header_to_write.serialize(out))
            {
                return false;
            }

            bool success = true;
            std::apply([&](const auto&... args)
                {
                    auto serialize_if_successful = [&](const auto& arg_to_serialize)
                        {
                            if (success)
                            {
                                success = core::binaryserializer::serialize_object(out, arg_to_serialize);
                            }
                        }; (serialize_if_successful(args), ...);
                }, getMembersTupleConst());

            if (!success)
            {
                return false;
            }
            return true;
        }

        bool ThemeConfig::deserialize(cyanvne::core::stream::InStreamInterface& in)
        {
            if (!in.is_open())
            {
                return false;
            }

            ThemeConfigFileHeader default_header_for_comparison;
            ThemeConfigFileHeader header_read_from_file;

            if (!header_read_from_file.deserialize(in))
            {
                return false;
            }

            constexpr double epsilon = 1e-7;
            if (std::abs(header_read_from_file.version - default_header_for_comparison.version) > epsilon ||
                header_read_from_file.magic != default_header_for_comparison.magic)
            {
                return false;
            }

            bool success = true;
            std::apply([&](auto&... args)
                {
                    auto deserialize_if_successful = [&](auto& arg_to_deserialize)
                        {
                        if (success)
                        {
                        success = core::binaryserializer::deserialize_object(in, arg_to_deserialize);
                        }
                        }; (deserialize_if_successful(args), ...);
                }, getMembersTuple());

            if (!success)
            {
                return false;
            }
            return true;
        }
    }
}