#pragma once
#include <string>
#include <vector>
#include <map>
#include <memory>
#include <optional>
#include <yaml-cpp/yaml.h>
#include <Core/Stream/Stream.h>
#include <Core/Serialization/Serialization.h>
#include <Resources/ThemeMinimumRequire/ThemeMinimumRequire.h>
#include <Core/Logger/Logger.h>

namespace cyanvne
{
    namespace resources
    {
        namespace themepack
        {
            class ThemePackCreator
            {
            private:
                std::shared_ptr<core::stream::OutStreamInterface> new_pack_stream_;
                std::shared_ptr<core::stream::InStreamInterface> yaml_config_stream_;

                std::shared_ptr<spdlog::logger> logger_ = core::GlobalLogger::getCoreLogger();
            public:
                ThemePackCreator(const std::shared_ptr<core::stream::InStreamInterface>& yaml_config_path,
                    const std::shared_ptr<core::stream::OutStreamInterface>& new_pack_stream);

                ThemePackCreator(const ThemePackCreator&) = delete;
                ThemePackCreator& operator=(const ThemePackCreator&) = delete;
                ThemePackCreator(ThemePackCreator&&) = delete;
                ThemePackCreator& operator=(ThemePackCreator&&) = delete;

                void directCreate();
                bool validateCreate(const themepack::GetThemeDataRequirementsInterface& requirements);
                void mergeCreate(const themepack::GetThemeDataRequirementsInterface& requirements, core::stream::InStreamInterface& old_theme_stream);

                ~ThemePackCreator() = default;
            };
        }
    }
}