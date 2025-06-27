#include "ThemeResourcesPacker.h"
#include "Core/MemoryStreamImpl/MemoryStreamImpl.h"
#include <variant>

namespace cyanvne
{
    namespace resources
    {
        void ThemeResourcesPacker::packThemeEntire(const parser::theme::ThemeConfig& theme_config,
            const parser::theme::ThemeGeneratorConfig& theme_generator_config) const
        {
            const auto buf_stream = std::make_shared<core::stream::DynamicMemoryStreamImpl>();
            if (theme_config.serialize(*buf_stream) < 0)
            {
                throw exception::resourcesexception::ThemeResourcePackerIOException("Failed to serialize theme config.");
            }

            packer_->addResourceByStream(buf_stream, ResourceType::CONFIG_DATA, "theme_config");

            if (theme_config.enable_built_in_font == true)
            {
                auto it = theme_generator_config.resources.find("built_in_font");
                if (it == theme_generator_config.resources.end())
                {
                    throw exception::resourcesexception::ThemeResourcePackerIOException(
                        "Resource 'built_in_font' is defined in ThemeConfig but has no path in ThemeGeneratorConfig.");
                }

                packer_->addResourceByStream(path_to_stream_->getInStream(it->second), ResourceType::FONT, "built_in_font");
            }
            
            for (const auto& [logical_name, theme_resource] : theme_config.resources)
            {
                auto it = theme_generator_config.resources.find(logical_name);
                if (it == theme_generator_config.resources.end())
                {
                    throw exception::resourcesexception::ThemeResourcePackerIOException(
                        "Resource '" + logical_name + "' is defined in ThemeConfig but has no path in ThemeGeneratorConfig.");
                }

                const std::string& source_path = it->second;
                const std::string& resource_alias = theme_resource.key;

                packer_->addResourceByStream(path_to_stream_->getInStream(source_path), ResourceType::IMAGE, resource_alias);
            }
        }

        void ThemeResourcesPacker::packThemeMerge(
            const parser::theme::ThemeConfig& target_theme_config,
            const parser::theme::ThemeGeneratorConfig& current_generator_config,
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

            auto buf_stream = std::make_shared<core::stream::DynamicMemoryStreamImpl>();
            if (target_theme_config.serialize(*buf_stream) < 0)
            {
                throw exception::resourcesexception::ThemeResourcePackerIOException("Failed to serialize target theme config.");
            }
            packer_->addResourceByStream(buf_stream, ResourceType::CONFIG_DATA, "theme_config");

            for (const auto& [logical_name, theme_resource] : target_theme_config.resources)
            {
                const std::string& resource_alias = theme_resource.key;
                
                ResourceType type = (logical_name == "BasicFont") ? ResourceType::FONT : ResourceType::IMAGE;
                
                auto it = current_generator_config.resources.find(logical_name);

                if (it != current_generator_config.resources.end())
                {
                    const std::string& source_path = it->second;
                    packer_->addResourceByStream(path_to_stream_->getInStream(source_path), type, resource_alias);
                }
                else
                {
                    try
                    {
                        const auto& old_theme_config = existing_theme_manager->getThemeConfig();
                        auto old_res_it = old_theme_config.resources.find(logical_name);
                        if (old_res_it == old_theme_config.resources.end())
                        {
                            throw exception::resourcesexception::ThemeResourcePackerIOException(
                                "Resource '" + logical_name + "' is expected in the new theme but was not found in the existing theme.");
                        }
                        const std::string& old_alias = old_res_it->second.key;

                        DataHandle handle = existing_theme_manager->getData(old_alias);

                        packer_->addResourceByData(handle->data, type, resource_alias);
                    }
                    catch (const std::exception& e)
                    {
                        throw exception::resourcesexception::ThemeResourcePackerIOException(
                            "Failed to merge resource '" + logical_name + "' from existing theme: " + std::string(e.what()));
                    }
                }
            }
        }

        void ThemeResourcesPacker::finalizePack()
        {
            if (packer_)
            {
                packer_->finalizePack();
            }
        }
    }
}
