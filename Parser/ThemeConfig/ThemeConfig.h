#pragma once
#include <string>
#include <vector>
#include <cstdint>
#include <cstddef>
#include <Core/Serialization/Serialization.h>
#include <Core/Stream/Stream.h>
#include <tuple>
#include <cmath>

namespace cyanvne
{
    namespace parser
    {
        struct ThemeConfig : public core::binaryserializer::BinarySerialiable
        {
            struct ThemeConfigFileHeader
            {
                double version = 0.1;
                uint64_t magic = 0xADAA2A4A6EF14F4EULL;

                std::ptrdiff_t serialize(core::stream::OutStreamInterface& out) const;
                std::ptrdiff_t deserialize(core::stream::InStreamInterface& in);
            };

            std::string name;

            bool enable_built_in_font = false;
            std::string basic_font_key;

            bool enable_main_menu_icon = false;
            std::string main_menu_icon_key;
            int32_t main_menu_icon_ms_per_frame = 0;
            std::vector<std::vector<uint32_t>> main_menu_icon_sprite_sheet;
            std::vector<double> main_menu_icon_rendering_area_scale;

            bool enable_background = false;
            std::string background_key;
            int32_t background_ms_per_frame = 0;
            std::vector<std::vector<uint32_t>> background_sprite_sheet;
            std::vector<double> background_rendering_area_scale;

            bool enable_create_cyan_data = false;
            std::string create_cyan_data_key;
            int32_t create_cyan_data_ms_per_frame = 0;
            std::vector<std::vector<uint32_t>> create_cyan_data_sprite_sheet;
            std::vector<double> create_cyan_data_rendering_area_scale;

            bool enable_create_cyan_theme = false;
            std::string create_cyan_theme_key;
            int32_t create_cyan_theme_ms_per_frame = 0;
            std::vector<std::vector<uint32_t>> create_cyan_theme_sprite_sheet;
            std::vector<double> create_cyan_theme_rendering_area_scale;

            bool enable_load_cyan_data = false;
            std::string load_cyan_data_key;
            int32_t load_cyan_data_ms_per_frame = 0;
            std::vector<std::vector<uint32_t>> load_cyan_data_sprite_sheet;
            std::vector<double> load_cyan_data_rendering_area_scale;

            bool enable_settings = false;
            std::string settings_key;
            int32_t settings_ms_per_frame = 0;
            std::vector<std::vector<uint32_t>> settings_sprite_sheet;
            std::vector<double> settings_rendering_area_scale;

            bool enable_close = false;
            std::string close_key;
            int32_t close_ms_per_frame = 0;
            std::vector<std::vector<uint32_t>> close_sprite_sheet;
            std::vector<double> close_rendering_area_scale;

            bool enable_dialog_image = false;
            std::string dialog_image_key;
            int32_t dialog_image_ms_per_frame = 0;
            std::vector<std::vector<uint32_t>> dialog_image_sprite_sheet;
            std::vector<double> dialog_image_rendering_area_scale;

            bool enable_save_process = false;
            std::string save_process_key;
            int32_t save_process_ms_per_frame = 0;
            std::vector<std::vector<uint32_t>> save_process_sprite_sheet;
            std::vector<double> save_process_rendering_area_scale;

            bool enable_load_process = false;
            std::string load_process_key;
            int32_t load_process_ms_per_frame = 0;
            std::vector<std::vector<uint32_t>> load_process_sprite_sheet;
            std::vector<double> load_process_rendering_area_scale;

            bool enable_history = false;
            std::string history_key;
            int32_t history_ms_per_frame = 0;
            std::vector<std::vector<uint32_t>> history_sprite_sheet;
            std::vector<double> history_rendering_area_scale;

            bool enable_setting_in_game = false;
            std::string setting_in_game_key;
            int32_t setting_in_game_ms_per_frame = 0;
            std::vector<std::vector<uint32_t>> setting_in_game_sprite_sheet;
            std::vector<double> setting_in_game_rendering_area_scale;

            bool enable_hide_dialog = false;
            std::string hide_dialog_key;
            int32_t hide_dialog_ms_per_frame = 0;
            std::vector<std::vector<uint32_t>> hide_dialog_sprite_sheet;
            std::vector<double> hide_dialog_rendering_area_scale;

            std::ptrdiff_t serialize(cyanvne::core::stream::OutStreamInterface& out) const override;
            std::ptrdiff_t deserialize(cyanvne::core::stream::InStreamInterface& in) override;

        private:
            auto getMembersTuple();
            auto getMembersTupleConst() const;
        };

        struct ThemeGeneratorConfig
        {
            bool enable_built_in_font;
            std::string basic_built_in_font_key;
            std::string basic_built_in_font_path;

            bool enable_main_menu_logo;
            std::string main_menu_logo_key;
            std::string main_menu_logo_path;

            bool enable_background;
            std::string background_key;
            std::string background_path;

            bool enable_create_cyan_data;
            std::string create_cyan_data_key;
            std::string create_cyan_data_path;

            bool enable_create_cyan_theme;
            std::string create_cyan_theme_key;
            std::string create_cyan_theme_path;

            bool enable_load_cyan_data;
            std::string load_cyan_data_key;
            std::string load_cyan_data_path;

            bool enable_settings;
            std::string settings_key;
            std::string settings_path;

            bool enable_close;
            std::string close_key;
            std::string close_path;

            bool enable_dialog_image;
            std::string dialog_image_key;
            std::string dialog_image_path;

            bool enable_save_process;
            std::string save_process_key;
            std::string save_process_path;

            bool enable_load_process;
            std::string load_process_key;
            std::string load_process_path;

            bool enable_history;
            std::string history_key;
            std::string history_path;

            bool enable_setting_in_game;
            std::string setting_in_game_key;
            std::string setting_in_game_path;

            bool enable_hide_dialog;
            std::string hide_dialog_key;
            std::string hide_dialog_path;
        };
    }
}