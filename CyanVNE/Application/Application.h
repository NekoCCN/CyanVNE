#pragma once
#include <Platform/WMContext/WMContext.h>
#include <Platform/WindowContext/WindowContext.h>
#include <Platform/GuiContext/GuiContext.h>
#include <Parser/AppSettings/AppSettings.h>
#include <Parser/ParserFactories/ParserFactories.h>
#include <Platform/EventBus/EventBus.h>
#include <Platform/Monitor/Monitor.h>
#include <Core/PathToStream/PathToStream.h>
#include <memory>
#include <string>
#include <Core/Stream/Stream.h>
#include <Resources/ThemeResourcesManager/ThemeResourcesManager.h>
#include <Runtime/GameStateManager/GameStateManager.h>
#include <Runtime/GuiDebugState/GuiDebugState.h>

namespace cyanvne
{
	class Application
	{
	private:
		bool is_initialized_ = false;

		parser::appsettings::AppSettings app_settings_;

		std::shared_ptr<platform::WMInitializer> wm_context_;
		std::shared_ptr<platform::WindowContext> window_context_;
		std::shared_ptr<platform::GuiContext> gui_context_;

		std::shared_ptr<platform::EventBus> event_bus_;

		std::shared_ptr<resources::ThemeResourcesManager> theme_resources_;

		std::shared_ptr<core::IPathToStream> path_to_stream_;

		std::shared_ptr<runtime::GameStateManager> game_state_manager_;
	public:
		Application(const std::shared_ptr<core::stream::InStreamInterface>& in_stream,
			const std::shared_ptr<core::IPathToStream>& path_to_stream) : path_to_stream_(path_to_stream)
		{
			if (in_stream)
			{
				try
				{
					app_settings_ = parser::factories::loadAppSettingsFromStream(*in_stream);
				}
				catch (const std::exception& e)
				{
					SDL_ShowSimpleMessageBox(SDL_MESSAGEBOX_ERROR, "CyanVNE Warn",
						(std::string("Use default app settings : ") + e.what()).c_str(), nullptr);
				}
			}
			else
			{
				SDL_ShowSimpleMessageBox(SDL_MESSAGEBOX_ERROR, "CyanVNE Warn",
					"AppSettings file is not found, Use default app settings" , nullptr);
			}

			core::GlobalLogger::initUniversalCoreLogger(app_settings_.logger);
			core::GlobalLogger::initUniversalClientLogger(app_settings_.logger);

			core::GlobalLogger::getCoreLogger()->info("CyanVNE is starting...");

			try
			{
				wm_context_ = std::make_shared<platform::WMInitializer>();
			}
			catch (exception::platformexception::InitWMSystemException& e)
			{
				SDL_ShowSimpleMessageBox(SDL_MESSAGEBOX_ERROR, "CyanVNE FATAL Error",
					"Failed to initialize WM system", nullptr);
				return;
			}

			try
			{
				if (app_settings_.window.is_ratio_window)
				{
					platform::MonitorLists monitors;
					if (monitors.getMonitorNum() < 1)
					{
						core::GlobalLogger::getCoreLogger()->warn("No monitor found, Use default windows size.");
						window_context_ = std::make_shared<platform::WindowContext>(
							app_settings_.title.c_str(), 1280, 720);
					}
					else if (monitors.getMonitorNum() >= 1)
					{
						double new_width = monitors.getVaildMonitorBounds().at(0).w * 0.8;
						double new_height = monitors.getVaildMonitorBounds().at(0).h * 0.7;
						core::GlobalLogger::getCoreLogger()->info("Try use ratio windows : {:.3f} x {:.3f}", new_width, new_height);
						window_context_ = std::make_shared<platform::WindowContext>(
							app_settings_.title.c_str(), new_width, new_height);
					}
				}
				else
				{
					core::GlobalLogger::getCoreLogger()->info("Create Window using the size specified in the configuration file {:d} x {:d}",
						app_settings_.window.width, app_settings_.window.height);
					window_context_ = std::make_shared<platform::WindowContext>(
						app_settings_.title.c_str(), app_settings_.window.width, app_settings_.window.height);
				}
				window_context_->setRenderVSync(1);
			}
			catch (std::exception& e)
			{
                core::GlobalLogger::getCoreLogger()->critical("Fail to init window context : {:s}", e.what());
				SDL_ShowSimpleMessageBox(SDL_MESSAGEBOX_ERROR, "Fail to init window context.", e.what(), nullptr);
			}


			std::shared_ptr<core::stream::InStreamInterface> theme_resources_stream =
				path_to_stream_->getInStream(app_settings_.theme_pack_path);
			if (theme_resources_stream == nullptr)
			{
				core::GlobalLogger::getCoreLogger()->critical("Failed to open theme pack file");
				SDL_ShowSimpleMessageBox(SDL_MESSAGEBOX_ERROR, "CyanVNE FATAL Error",
					"Failed to open theme pack file", nullptr);
				return;
			}

			try
			{
				std::shared_ptr<resources::ResourcesManager> theme_resources_manager =
					std::make_shared<resources::ResourcesManager>(theme_resources_stream);
				theme_resources_ = std::make_shared<resources::ThemeResourcesManager>(theme_resources_manager,
					app_settings_.caching.theme_caching_config.max_volatile_size,
					app_settings_.caching.theme_caching_config.max_persistent_size,
					app_settings_.caching.theme_caching_config.max_single_persistent_size
				);
			}
			catch (const exception::resourcesexception::ResourceManagerIOException& e)
			{
				core::GlobalLogger::getCoreLogger()->critical("Failed to open theme pack file : {:s}", e.what());
				SDL_ShowSimpleMessageBox(SDL_MESSAGEBOX_ERROR, "CyanVNE FATAL Error",
					"Failed to open theme pack file", nullptr);
				return;
			}

            if (theme_resources_->isInitialized())
            {
                core::GlobalLogger::getCoreLogger()->info("Theme pack file opened");
            }
            else
            {
                core::GlobalLogger::getCoreLogger()->critical("Failed to open theme pack file");
                SDL_ShowSimpleMessageBox(SDL_MESSAGEBOX_ERROR, "CyanVNE FATAL Error",
                    "Failed to open theme pack file", nullptr);
                return;
            }

			gui_context_ = app_settings_.languages.is_enabled ?
				platform::GuiContext::create(window_context_,
					               theme_resources_->getResourceData("built_in_font"), 30, app_settings_.languages.supported_languages)
				:
				platform::GuiContext::create(window_context_,
					               theme_resources_->getResourceData("built_in_font"), 30);

			event_bus_ = std::make_shared<platform::EventBus>();

            game_state_manager_ = std::make_shared<runtime::GameStateManager>(window_context_, event_bus_);

			game_state_manager_->pushState(std::make_unique<runtime::GuiDebugState>());

			is_initialized_ = true;
		}

		int start() const
		{
			if (!is_initialized_)
			{
				return -1;
			}
			core::GlobalLogger::getCoreLogger()->info("Application main loop starting...");
			Uint64 last_time = SDL_GetTicks();

			SDL_Event e;
			int32_t status = 1;

			while (status == 1)
			{
				Uint64 current_time = SDL_GetTicks();
				float delta_time = (current_time - last_time) / 1000.0f;

				delta_time = std::min(delta_time, 0.1f);

				last_time = current_time;

				while (SDL_PollEvent(&e))
				{
                    event_bus_->processAndPublishSDL(e, ImGui::GetIO());

					if (e.type == SDL_EVENT_QUIT)
					{
						status = 0;
					}
				}

				event_bus_->dispatchPendingEvents();

				game_state_manager_->updateCurrentState(delta_time);


				window_context_->setRenderDrawColorInt(255, 255, 255, 255);
				window_context_->renderClear();

				ImGui_ImplSDLRenderer3_NewFrame();
				ImGui_ImplSDL3_NewFrame();
				ImGui::NewFrame();

				game_state_manager_->renderCurrentStates();


				ImGui::Render();
				ImGui_ImplSDLRenderer3_RenderDrawData(ImGui::GetDrawData(), window_context_->getRendererHinding());

				window_context_->presentRender();
			}

			core::GlobalLogger::getCoreLogger()->info("Application main loop finished.");

			return status;
		}
	};
}