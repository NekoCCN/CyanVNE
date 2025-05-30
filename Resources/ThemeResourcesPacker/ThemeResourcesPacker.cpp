#include "ThemeResourcesPacker.h"
#include <Core/MemoryStreamImpl/MemoryStreamImpl.h>

void cyanvne::resources::ThemeResourcesPacker::packThemeEntire(const parser::ThemeConfig& theme_config,
	const parser::ThemeGeneratorConfig& theme_generator_config) const
{
	std::shared_ptr<core::stream::DynamicMemoryStreamImpl> buf_stream_ =
		std::make_shared<core::stream::DynamicMemoryStreamImpl>();
	if (theme_config.serialize(*buf_stream_) < 0)
	{
		throw exception::resourcesexception::ThemeResourcePackerIOException ("Failed to serialize theme config.");
	}

	packer_->addResourceByStream(buf_stream_, ResourceType::CONFIG_DATA, "theme_config");

	if (theme_config.enable_built_in_font != theme_generator_config.enable_built_in_font)
	{
        throw exception::IllegalArgumentException("theme_config.enable_built_in_font != theme_generator_config.enable_built_in_font");
	}

    if (theme_config.enable_built_in_font)
    {
		packer_->addResourceByStream(path_to_stream_->getInStream(theme_generator_config.basic_built_in_font_path)
			, ResourceType::FONT, "built_in_font");
    }


	auto packElement = [&](bool themeconfig_is_enable, bool themegenerator_config_is_enable,
		const std::string& key, const std::string& path)
	{
		if (themeconfig_is_enable != themegenerator_config_is_enable)
		{
			throw exception::IllegalArgumentException("themeconfig_is_enable != themegenerator_config_is_enable");
		}
		if (themeconfig_is_enable == false)
		{
			return;
		}
		packer_->addResourceByStream(path_to_stream_->getInStream(path), ResourceType::IMAGE, key);
	};

    packElement(theme_config.enable_main_menu_icon, theme_generator_config.enable_main_menu_icon,
        theme_config.main_menu_icon_key, theme_generator_config.main_menu_icon_path);
    packElement(theme_config.enable_background, theme_generator_config.enable_background,
        theme_config.background_key, theme_generator_config.background_path);
    packElement(theme_config.enable_create_cyan_data, theme_generator_config.enable_create_cyan_data,
        theme_config.create_cyan_data_key, theme_generator_config.create_cyan_data_path);
    packElement(theme_config.enable_create_cyan_theme, theme_generator_config.enable_create_cyan_theme,
        theme_config.create_cyan_theme_key, theme_generator_config.create_cyan_theme_path);
    packElement(theme_config.enable_load_cyan_data, theme_generator_config.enable_load_cyan_data,
        theme_config.load_cyan_data_key, theme_generator_config.load_cyan_data_path);
    packElement(theme_config.enable_settings, theme_generator_config.enable_settings,
        theme_config.settings_key, theme_generator_config.settings_path);
    packElement(theme_config.enable_close, theme_generator_config.enable_close,
        theme_config.close_key, theme_generator_config.close_path);
    packElement(theme_config.enable_dialog_image, theme_generator_config.enable_dialog_image,
        theme_config.dialog_image_key, theme_generator_config.dialog_image_path);
    packElement(theme_config.enable_save_process, theme_generator_config.enable_save_process,
        theme_config.save_process_key, theme_generator_config.save_process_path);
    packElement(theme_config.enable_load_process, theme_generator_config.enable_load_process,
        theme_config.load_process_key, theme_generator_config.load_process_path);
    packElement(theme_config.enable_history, theme_generator_config.enable_history,
        theme_config.history_key, theme_generator_config.history_path);
    packElement(theme_config.enable_setting_in_game, theme_generator_config.enable_setting_in_game,
        theme_config.setting_in_game_key, theme_generator_config.setting_in_game_path);
    packElement(theme_config.enable_hide_dialog, theme_generator_config.enable_hide_dialog,
        theme_config.hide_dialog_key, theme_generator_config.hide_dialog_path);
}

void cyanvne::resources::ThemeResourcesPacker::packThemeMerge(const parser::ThemeConfig& target_theme_config,
	const parser::ThemeGeneratorConfig& current_generator_config,
	const std::shared_ptr<ThemeResourcesManager>& existing_theme_manager) const
{
	if (!packer_)
	{
		throw exception::IllegalStateException("Internal packer is not initialized.");
	}
	if (!existing_theme_manager || !existing_theme_manager->isInitialized())
	{
		throw exception::IllegalArgumentException("Existing ThemeResourcesManager is null or not initialized.");
	}

	std::shared_ptr<core::stream::DynamicMemoryStreamImpl> buf_stream_ =
		std::make_shared<core::stream::DynamicMemoryStreamImpl>();
	if (target_theme_config.serialize(*buf_stream_) < 0)
	{
		throw exception::resourcesexception::ThemeResourcePackerIOException("Failed to serialize target theme config.");
	}
	packer_->addResourceByStream(buf_stream_, ResourceType::CONFIG_DATA, "theme_config");

	if (target_theme_config.enable_built_in_font != current_generator_config.enable_built_in_font)
	{
		throw exception::IllegalArgumentException(
			"target_theme_config.enable_built_in_font != current_generator_config.enable_built_in_font");
	}
	if (target_theme_config.enable_built_in_font)
	{
		const std::string& font_path = current_generator_config.basic_built_in_font_path;
		if (!font_path.empty())
		{
			std::shared_ptr<core::stream::InStreamInterface> font_stream =
				path_to_stream_->getInStream(font_path);
			if (!font_stream || !font_stream->is_open())
			{
				throw exception::resourcesexception::ThemeResourcePackerIOException(
					"Failed to open built-in font stream from path: " + font_path);
			}
			packer_->addResourceByStream(font_stream, ResourceType::FONT, "built_in_font");
		}
		else
		{
			try
			{
				std::vector<uint8_t> font_data = existing_theme_manager->getResourceDataByAlias("built_in_font");
				packer_->addResourceByData(font_data, ResourceType::FONT, "built_in_font");
			}
			catch (const exception::CyanVNEIOException& e)
			{
				throw exception::resourcesexception::ThemeResourcePackerIOException(
					"Built-in font path is empty and resource 'built_in_font' not found in existing theme manager: " + std::string(e.what()));
			}
		}
	}

	auto mergeElement = [&](bool target_config_is_enable, bool generator_config_is_enable,
	                        const std::string& key, const std::string& path_from_generator)
	{
		if (target_config_is_enable != generator_config_is_enable)
		{
			throw exception::IllegalArgumentException(
				"Enable flag mismatch for resource key: " + key +
				" (TargetThemeConfig: " + (target_config_is_enable ? "true" : "false") +
				", GeneratorConfig: " + (generator_config_is_enable ? "true" : "false") + ")"
			);
		}
		if (target_config_is_enable == false)
		{
			return;
		}

		if (!path_from_generator.empty())
		{
			std::shared_ptr<core::stream::InStreamInterface> element_stream =
				path_to_stream_->getInStream(path_from_generator);
			if (!element_stream || !element_stream->is_open())
			{
				throw exception::resourcesexception::ThemeResourcePackerIOException(
					"Failed to open stream for resource key '" + key + "' at path: " + path_from_generator);
			}
			packer_->addResourceByStream(element_stream, ResourceType::IMAGE, key);
		}
		else
		{
			try
			{
				std::vector<uint8_t> element_data = existing_theme_manager->getResourceDataByAlias(key);
				packer_->addResourceByData(element_data, ResourceType::IMAGE, key);
			}
			catch (const exception::CyanVNEIOException& e)
			{
				throw exception::resourcesexception::ThemeResourcePackerIOException(
					"Resource path is empty for key '" + key + "' and it was not found in the existing theme manager: " + std::string(e.what()));
			}
		}
	};

	mergeElement(target_theme_config.enable_main_menu_icon, current_generator_config.enable_main_menu_icon,
	             target_theme_config.main_menu_icon_key, current_generator_config.main_menu_icon_path);
	mergeElement(target_theme_config.enable_background, current_generator_config.enable_background,
	             target_theme_config.background_key, current_generator_config.background_path);
	mergeElement(target_theme_config.enable_create_cyan_data, current_generator_config.enable_create_cyan_data,
	             target_theme_config.create_cyan_data_key, current_generator_config.create_cyan_data_path);
	mergeElement(target_theme_config.enable_create_cyan_theme, current_generator_config.enable_create_cyan_theme,
	             target_theme_config.create_cyan_theme_key, current_generator_config.create_cyan_theme_path);
	mergeElement(target_theme_config.enable_load_cyan_data, current_generator_config.enable_load_cyan_data,
	             target_theme_config.load_cyan_data_key, current_generator_config.load_cyan_data_path);
	mergeElement(target_theme_config.enable_settings, current_generator_config.enable_settings,
	             target_theme_config.settings_key, current_generator_config.settings_path);
	mergeElement(target_theme_config.enable_close, current_generator_config.enable_close,
	             target_theme_config.close_key, current_generator_config.close_path);
	mergeElement(target_theme_config.enable_dialog_image, current_generator_config.enable_dialog_image,
	             target_theme_config.dialog_image_key, current_generator_config.dialog_image_path);
	mergeElement(target_theme_config.enable_save_process, current_generator_config.enable_save_process,
	             target_theme_config.save_process_key, current_generator_config.save_process_path);
	mergeElement(target_theme_config.enable_load_process, current_generator_config.enable_load_process,
	             target_theme_config.load_process_key, current_generator_config.load_process_path);
	mergeElement(target_theme_config.enable_history, current_generator_config.enable_history,
	             target_theme_config.history_key, current_generator_config.history_path);
	mergeElement(target_theme_config.enable_setting_in_game, current_generator_config.enable_setting_in_game,
	             target_theme_config.setting_in_game_key, current_generator_config.setting_in_game_path);
	mergeElement(target_theme_config.enable_hide_dialog, current_generator_config.enable_hide_dialog,
	             target_theme_config.hide_dialog_key, current_generator_config.hide_dialog_path);
}

void cyanvne::resources::ThemeResourcesPacker::finalizePack()
{
    packer_->finalizePack();
}
