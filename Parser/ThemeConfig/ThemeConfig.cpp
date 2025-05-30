#include "ThemeConfig.h"
#include <tuple>
#include <cmath>

namespace cyanvne
{
    namespace parser
    {
        std::ptrdiff_t ThemeConfig::ThemeConfigFileHeader::serialize(core::stream::OutStreamInterface& out) const
        {
            std::ptrdiff_t total_bytes_written = 0;

            std::ptrdiff_t bytes_written = core::binaryserializer::serialize_object(out, version);
            if (bytes_written == -1)
            {
                return -1;
            }
            total_bytes_written += bytes_written;

            bytes_written = core::binaryserializer::serialize_object(out, magic);
            if (bytes_written == -1)
            {
                return -1;
            }
            total_bytes_written += bytes_written;

            return total_bytes_written;
        }

        std::ptrdiff_t ThemeConfig::ThemeConfigFileHeader::deserialize(core::stream::InStreamInterface& in)
        {
            std::ptrdiff_t total_bytes_read = 0;

            std::ptrdiff_t bytes_read = core::binaryserializer::deserialize_object(in, version);
            if (bytes_read == -1)
            {
                return -1;
            }
            total_bytes_read += bytes_read;

            bytes_read = core::binaryserializer::deserialize_object(in, magic);
            if (bytes_read == -1)
            {
                return -1;
            }
            total_bytes_read += bytes_read;

            return total_bytes_read;
        }

        auto ThemeConfig::getMembersTuple()
        {
            return std::tie(
                name, enable_built_in_font, built_in_font_key,
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
                name, enable_built_in_font, built_in_font_key,
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

        std::ptrdiff_t ThemeConfig::serialize(cyanvne::core::stream::OutStreamInterface& out) const
        {
            std::ptrdiff_t total_bytes_written = 0;

            ThemeConfigFileHeader header_to_write;
            std::ptrdiff_t bytes_written = header_to_write.serialize(out);
            if (bytes_written == -1)
            {
                return -1;
            }
            total_bytes_written += bytes_written;

            bool Succeeded = true;
            std::apply(
                [&](const auto&... args)
                {
                    auto serialize_member =
                        [&](const auto& member_to_serialize)
                        {
                            if (!Succeeded)
                            {
                                return;
                            }
                            std::ptrdiff_t current_bytes_written = core::binaryserializer::serialize_object(out, member_to_serialize);
                            if (current_bytes_written == -1)
                            {
                                Succeeded = false;
                                return;
                            }
                            total_bytes_written += current_bytes_written;
                        };
                    (serialize_member(args), ...);
                }, getMembersTupleConst());

            if (!Succeeded)
            {
                return -1;
            }

            return total_bytes_written;
        }

        std::ptrdiff_t ThemeConfig::deserialize(cyanvne::core::stream::InStreamInterface& in)
        {
            std::ptrdiff_t total_bytes_read = 0;

            ThemeConfigFileHeader default_header_for_comparison;
            ThemeConfigFileHeader header_read_from_file;

            std::ptrdiff_t bytes_read = header_read_from_file.deserialize(in);
            if (bytes_read == -1)
            {
                return -1;
            }
            total_bytes_read += bytes_read;

            constexpr double epsilon = 1e-7;
            if (std::abs(header_read_from_file.version - default_header_for_comparison.version) > epsilon ||
                header_read_from_file.magic != default_header_for_comparison.magic)
            {
                return -1;
            }

            bool Succeeded = true;
            std::apply(
                [&](auto&... args)
                {
                    auto deserialize_member =
                        [&](auto& member_to_deserialize)
                        {
                            if (!Succeeded)
                            {
                                return;
                            }
                            std::ptrdiff_t current_bytes_read = core::binaryserializer::deserialize_object(in, member_to_deserialize);
                            if (current_bytes_read == -1)
                            {
                                Succeeded = false;
                                return;
                            }
                            total_bytes_read += current_bytes_read;
                        };
                    (deserialize_member(args), ...);
                }, getMembersTuple());

            if (!Succeeded)
            {
                return -1;
            }

            return total_bytes_read;
        }
    }
}