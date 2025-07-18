﻿#include "CyanVNE.h"
#include "Resources/StreamUniversalImpl/StreamUniversalImpl.h"
#include "Resources/UniversalPathToStream/UniversalPathToStream.h"
#include <Resources/ThemeResourcesPacker/ThemeResourcesPacker.h>
#include <Parser/ThemeConfigParser/ThemeConfigParser.h>
#include <Core/Stream/Stream.h>
#include <entt/entt.hpp>

int main()
{
	entt::registry registry;

	std::shared_ptr<cyanvne::resources::UniversalPathToStream> path_to_stream =
		std::make_shared<cyanvne::resources::UniversalPathToStream>();

	cyanvne::Application app(path_to_stream->getInStream(R"(E:\CyanVNE-TestProject\AppSettings.yaml)"), path_to_stream);

	//try {
	//	std::shared_ptr<cyanvne::core::stream::InStreamInterface> theme_generator_packer =
	//		path_to_stream->getInStream(R"(E:\CyanVNE-TestProject\MiniThemeGeneratorSetting.yaml)");

	//	cyanvne::resources::ThemeResourcesPacker theme_packer(path_to_stream, R"(E:\CyanVNE-TestProject\MiniTheme.cyantr)");
	//	theme_packer.packThemeEntire(cyanvne::parser::factories::loadThemeConfigFromStream(*theme_generator_packer)
	//	, cyanvne::parser::factories::loadThemeGeneratorConfigFromStream(*theme_generator_packer));
	//	theme_packer.finalizePack();
	//}
	//catch (cyanvne::exception::parserexception::ParserException& e)
	//{
	//	cyanvne::core::GlobalLogger::getCoreLogger()->error(e.what());
	//	cyanvne::core::GlobalLogger::getCoreLogger()->error(e.getFormatErrorMessage());
	//}

	return app.start();
}
