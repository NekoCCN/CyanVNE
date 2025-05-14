#pragma once
#include <vector>
#include <Core/TSVector/TSVector.h>
#include <Core/SpscQueue/SpscQueue.h>
#include <memory>
#include <Core/Serialization/Serialization.h>
#include <SDL3/SDL.h>
#include <Runtime/BasicRender/BasicRender.h>
#include <Runtime/RuntimeException/RuntimeException.h>
#include <Runtime/WindowContext/WindowContext.h>
#include <Runtime/GuiContext/GuiContext.h>

namespace cyanvne
{
	namespace runtime
	{
		class EventCore
		{
		private:
			std::shared_ptr<WindowContext> window_context_;
			std::shared_ptr<GuiContext> gui_context_;  // Is a signals of ImGui init successfully
			bool is_gui_responsive_ = true;

			core::ThreadSafeVector<std::shared_ptr<basicrender::ChangeableWindowSizeInterface>> when_changed_window_size_listeners_;
			core::ThreadSafeVector<std::shared_ptr<basicrender::SettableThemePackInterface>> when_set_theme_pack_listeners_;

			core::ThreadSafeVector<std::shared_ptr<basicrender::RenderableInterface>> resident_render_list_;
			core::ThreadSafeVector<std::shared_ptr<basicrender::ResponsiveInterface>> resident_universal_event_listeners_;

			core::spscqueue::SpscQueue<std::shared_ptr<basicrender::RenderableInterface>> render_queue_;
			core::spscqueue::SpscQueue<std::shared_ptr<basicrender::ResponsiveInterface>> universal_event_queue_;
		public:
			EventCore(const std::shared_ptr<WindowContext>& window_context, const std::shared_ptr<GuiContext>& gui_context,
				const int queue_size)
				: window_context_(window_context), gui_context_(gui_context),
				render_queue_(queue_size, core::spscqueue::SpscQueue<std::shared_ptr<basicrender::RenderableInterface>>::WaitType::Yield),
				universal_event_queue_(queue_size, core::spscqueue::SpscQueue<std::shared_ptr<basicrender::ResponsiveInterface>>::WaitType::Backoff)
			{
				if (window_context_ == nullptr)
					throw exception::runtimeexception::InitEventCoreException("EventCore::EventCore: window_context_ is nullptr");
				if (gui_context_ == nullptr)
					throw exception::runtimeexception::InitEventCoreException("EventCore::EventCore: gui_context_ is nullptr");
			}

			void processRender()
			{
				for (const std::shared_ptr<basicrender::RenderableInterface>& render_list
					: resident_render_list_)
				{
					render_list->render(window_context_);
				}

				std::shared_ptr<basicrender::RenderableInterface> render;
				if (render_queue_.dequeue(render, false) == true)
				{
					render->render(window_context_);
				}
			}

			void processEvent(const SDL_Event* event)
            { 
				for (const std::shared_ptr<basicrender::ResponsiveInterface>& responsive_list
					: resident_universal_event_listeners_)
				{
					responsive_list->response(event);
				}

				if (is_gui_responsive_ == true)
				{
					ImGui_ImplSDL3_ProcessEvent(event);
				}

				std::shared_ptr<basicrender::ResponsiveInterface> responsive;
                if (universal_event_queue_.dequeue(responsive, false) == true)
				{
					responsive->response(event);
				}
            }

			void pushRenderToQueue(const std::shared_ptr<basicrender::RenderableInterface>& render)
			{
				render_queue_.enqueue(render);
			}

			void pushResidentRender(const std::shared_ptr<basicrender::RenderableInterface>& render)
			{
				resident_render_list_.push_back(render);
			}

            void pushResponsiveToQueue(const std::shared_ptr<basicrender::ResponsiveInterface>& responsive)
            {
                universal_event_queue_.enqueue(responsive);
            }

            void pushResidentResponsive(const std::shared_ptr<basicrender::ResponsiveInterface>& responsive)
            {
                resident_universal_event_listeners_.push_back(responsive);
            }

			void popResidentRender()
			{
				resident_render_list_.pop_back();
			}

            void popResidentResponsive()
            {
                resident_universal_event_listeners_.pop_back();
            }

			void setGuiResponsive(bool gui_responsive)
			{
				is_gui_responsive_ = gui_responsive;
			}

            void addWhenChangedWindowSizeListener(const std::shared_ptr<basicrender::ChangeableWindowSizeInterface>& listener)
            {
                when_changed_window_size_listeners_.push_back(listener);
            }

            void addWhenSetThemePackListener(const std::shared_ptr<basicrender::SettableThemePackInterface>& listener)
            {
                when_set_theme_pack_listeners_.push_back(listener);
            }

			void changeWindowSize(int new_w, int new_h)
			{
                for (const std::shared_ptr<basicrender::ChangeableWindowSizeInterface>& listener
                    : when_changed_window_size_listeners_)
                {
                    listener->whenChangedWindowSize(new_w, new_h);
                }
			}

            //void setThemePack(const std::shared_ptr<basicrender::ThemePack>& theme_pack)
            //{
            //    for (const std::shared_ptr<basicrender::SettableThemePackInterface>& listener
            //        : when_set_theme_pack_listeners_)
            //    {
            //        listener->(theme_pack);
            //    }
            //}
		};
	}
}