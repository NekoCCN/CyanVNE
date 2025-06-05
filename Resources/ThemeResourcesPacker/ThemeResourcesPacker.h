#pragma once
#include <Resources/ResourcesPacker/ResourcesPacker.h>
#include <Parser/ThemeConfig/ThemeConfig.h>
#include <Core/PathToStream/PathToStream.h>
#include <Resources/ThemeResourcesManager/ThemeResourcesManager.h>

namespace cyanvne
{
	namespace resources
	{
		class ThemeResourcesPacker : public IResourcesPacker
		{
		private:
			std::shared_ptr<core::IPathToStream> path_to_stream_;

			std::shared_ptr<ResourcesPacker> packer_;
		public:
			ThemeResourcesPacker(const std::shared_ptr<core::IPathToStream>& path_to_stream, 
				const std::string& output_file_path) : path_to_stream_(path_to_stream),
				packer_(std::make_shared<ResourcesPacker>(path_to_stream->getOutStream(output_file_path)))
			{  }

			void packThemeEntire(const parser::ThemeConfig& theme_config,
				const parser::ThemeGeneratorConfig& theme_generator_config) const;

            void packThemeMerge(
                const parser::ThemeConfig& target_theme_config,
                const parser::ThemeGeneratorConfig& current_generator_config,
                const std::shared_ptr<ThemeResourcesManager>& existing_theme_manager) const;

			void finalizePack() override;
		};
	}
}
