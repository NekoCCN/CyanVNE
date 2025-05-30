#pragma once
#include <Platform/WMContext/WMContext.h>
#include <Platform/WindowContext/WindowContext.h>
#include <Platform/GuiContext/GuiContext.h>
#include <Parser/AppSettings/AppSettings.h>
#include <Platform/EventBus/EventBus.h>
#include <Platform/Monitor/Monitor.h>
#include <memory>
#include <string>
#include <Core/Stream/Stream.h>

namespace cyanvne
{
	class Application
	{
	private:
		parser::appsettings::AppSettings app_settings_;

        std::shared_ptr<platform::WMInitializer> wm_context_;
        std::shared_ptr<platform::WindowContext> window_context_;
        std::shared_ptr<platform::GuiContext> gui_context_;

		std::shared_ptr<platform::EventBus> event_bus_;
	public:
		Application(std::shared_ptr<core::stream::InStreamInterface> in_stream)
		{
			if (!in_stream)
			{
				cyanvne::parser::appsettings::AppSettingsParser app_settings_parser;
				try
				{
					app_settings_parser.parse(in_stream);
					app_settings_ = app_settings_parser.get();
				}
				catch (const std::exception& e)
				{
					SDL_ShowSimpleMessageBox(SDL_MESSAGEBOX_ERROR, "CyanVNE Warn",
						(std::string("Use default app settings : ") + e.what()).c_str(), nullptr);
				}
			}

			wm_context_ = std::make_shared<platform::WMInitializer>();

			core::GlobalLogger::initUniversalCoreLogger(app_settings_.logger_config_);
			core::GlobalLogger::initUniversalClientLogger(app_settings_.logger_config_);

			core::GlobalLogger::getCoreLogger()->info("CyanVNE is starting...");

			if (app_settings_.is_ratio_window)
			{
				core::GlobalLogger::getCoreLogger()->info("Try use ratio windows.");
				platform::MonitorLists monitors;
				if (monitors.getMonitorNum() < 1)
				{
					core::GlobalLogger::getCoreLogger()->error("No monitor found, Use default windows size.");
					window_context_ = std::make_shared<platform::WindowContext>(
						app_settings_.title, 1280, 720);
				}
				else if (monitors.getMonitorNum() >= 1)
				{
					window_context_ = std::make_shared<platform::WindowContext>(
						app_settings_.title, monitors.getMonitorBounds().at(0).w * 0.6,
						monitors.getMonitorBounds().at(0).h * 0.35);
				}
				
			}
			else
			{
				core::GlobalLogger::getCoreLogger()->info("Create Window using the size specified in the configuration file {:d} x {:d}",
					app_settings_.window_width, app_settings_.window_height);
                window_context_ = std::make_shared<platform::WindowContext>(
					app_settings_.title, app_settings_.window_width, app_settings_.window_height);
			}

			gui_context_ = std::make_shared<platform::GuiContext>();
		}
	};
}