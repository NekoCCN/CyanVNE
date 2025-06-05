#include "CyanVNE.h"
#include "Resources/StreamUniversalImpl/StreamUniversalImpl.h"
#include "Resources/UniversalPathToStream/UniversalPathToStream.h"
#include <Resources/ThemeResourcesPacker/ThemeResourcesPacker.h>
#include <Parser/ThemeConfigParser/ThemeConfigParser.h>
#include <Core/Stream/Stream.h>

int main()
{
	std::shared_ptr<cyanvne::resources::UniversalPathToStream> path_to_stream =
		std::make_shared<cyanvne::resources::UniversalPathToStream>();

	cyanvne::Application app(path_to_stream->getInStream(R"(E:\CyanVNE-TestProject\AppSettings.yaml)"), path_to_stream);

	//try {
	//	std::shared_ptr<cyanvne::core::stream::InStreamInterface> theme_generator_packer =
	//		path_to_stream->getInStream(R"(E:\CyanVNE-TestProject\MiniThemeGeneratorSetting.yaml)");
	//	cyanvne::parser::theme::ThemeConfigParser theme_parser;
	//	theme_parser.parse(theme_generator_packer);
	//	cyanvne::parser::theme::ThemeGeneratorConfigParser theme_generator_parser;
	//	theme_generator_parser.parse(theme_generator_packer);

	//	cyanvne::resources::ThemeResourcesPacker theme_packer(path_to_stream, R"(E:\CyanVNE-TestProject\MiniTheme.cyantr)");
	//	theme_packer.packThemeEntire(theme_parser.get(), theme_generator_parser.get());
	//	theme_packer.finalizePack();
	//}
	//catch (cyanvne::exception::parserexception::ParserException& e)
	//{
	//	cyanvne::core::GlobalLogger::getCoreLogger()->error(e.what());
	//	cyanvne::core::GlobalLogger::getCoreLogger()->error(e.getFormatErrorMessage());
	//}

	return app.start();
}
