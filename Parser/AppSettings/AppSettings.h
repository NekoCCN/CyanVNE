#pragma once
#include <string>
#include <set>
#include <Core/Logger/Logger.h>

namespace cyanvne
{
    namespace parser
    {
        namespace appsettings
        {
            struct DefaultLoadingConfig
            {
                bool is_enabled = false;
                std::string file;
            };

            struct WindowConfig
            {
                bool is_ratio_window = false;
                int width = 1024;
                int height = 612;
                bool is_fullscreen = false;
                bool is_windowed_fullscreen = true;
            };

            struct VsyncConfig
            {
                bool is_enabled = true;
                int fps = -1;
            };

            struct LanguageConfig
            {
                bool is_enabled = false;
                std::set<std::string> supported_languages;
            };

            struct CachingConfig
            {
                uint64_t max_volatile_size;
                uint64_t max_persistent_size;
                uint64_t max_single_persistent_size;
            };

            struct AppCachingConfig
            {
                CachingConfig theme_caching_config = { .max_volatile_size = 20971520,
                    .max_persistent_size = 10485760,
	                .max_single_persistent_size= 3145728
                };
                CachingConfig process_data_caching_config = { .max_volatile_size = 52428800, 
                    .max_persistent_size = 20971520,
	                .max_single_persistent_size = 5242880
                };
            };

            struct AppSettings
            {
                std::string title = "CyanVNE";
                std::string graphics_driver;
                std::string theme_pack_path = "BasicTheme.cyantr";
                bool is_default_font = false;

                DefaultLoadingConfig default_loading;
                WindowConfig window;
                VsyncConfig vsync;
                LanguageConfig languages;
                core::GlobalLogger::LoggerConfig logger;

                AppCachingConfig caching;
            };
        }
    }
}