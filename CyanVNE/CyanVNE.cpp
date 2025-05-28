#include "CyanVNE.h"
#include <Core/Logger/Logger.h>
#include <Platform/WMContext/WMContext.h>
#include <Platform/WindowContext/WindowContext.h>
#include <Platform/GuiContext/GuiContext.h>
#include <Parser/AppSettings/AppSettings.h>
#include <Resources/StreamUniversalImpl/StreamUniversalImpl.h>
#include <SDL3/SDL.h>

int main()
{
	try
	{
		cyanvne::platform::WMInitializer wmInit;
	}
    catch (const std::exception& e)
    {
		SDL_ShowSimpleMessageBox(SDL_MESSAGEBOX_ERROR, "CyanVNE Fatal Error",
			e.what(), nullptr);
		return -1;
    }
	
	std::shared_ptr<cyanvne::resources::InStreamUniversalImpl> in_stream =
		cyanvne::resources::InStreamUniversalImpl::createFromBinaryFile("AppSettings.yaml");
	cyanvne::parser::appsettings::AppSettings app_settings;

	if (!in_stream)
	{
		cyanvne::parser::appsettings::AppSettingsParser app_settings_parser;
		try
		{
			app_settings_parser.parse(in_stream);
			app_settings = app_settings_parser.get();
		}
        catch (const std::exception& e)
        {
			SDL_ShowSimpleMessageBox(SDL_MESSAGEBOX_ERROR, "CyanVNE Warn",
				e.what(), nullptr);
        }
	}

}
