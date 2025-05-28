#pragma once
#include <imgui.h>
#include <iostream>
#include <SDL3/SDL.h>
#include <Core/Logger/Logger.h>
#include "Platform/PlatformException/PlatformException.h"
#include <SDL3_ttf/SDL_ttf.h>
#ifdef IS_WIN32_SYS
#include <windows.h>
#endif

namespace cyanvne
{
	namespace platform
	{
		class WMInitializer
		{
		private:
			static void init_SDL()
			{
				if (!SDL_InitSubSystem(SDL_INIT_AUDIO | SDL_INIT_VIDEO | SDL_INIT_EVENTS | SDL_INIT_JOYSTICK | SDL_INIT_HAPTIC | SDL_INIT_GAMEPAD))
				{
					core::GlobalLogger::getCoreLogger()->critical("Window system initialization failed : {:s}", SDL_GetError());
					throw exception::platformexception::InitWMSystemException("SDL3 initialization failed");
				}
				else
				{
					core::GlobalLogger::getCoreLogger()->info("Window system initialization successful");
				}
			}

			static void init_SDL_TTF()
			{
				if (!TTF_Init())
				{
					core::GlobalLogger::getCoreLogger()->critical("TrueType system initialization failed : {:s}", SDL_GetError());
					throw exception::platformexception::InitWMSystemException("SDL3_TTF initialization failed");
				}
				else
				{
					core::GlobalLogger::getCoreLogger()->info("TrueType system initialization successful");
				}
			}

			static void init()
			{
				init_SDL();
				init_SDL_TTF();
			}
		public:
			WMInitializer()
			{
				init();
			}
			~WMInitializer()
			{
				TTF_Quit();
				SDL_Quit();
			}
		};
	}
}