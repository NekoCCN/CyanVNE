#include "EventBus.h"

void cyanvne::platform::Subscription::release()
{
    if (bus_ptr_ && id_ != 0)
    {
        if (is_sdl_subscription_)
        {
            bus_ptr_->unsubscribe_sdl(id_);
        }
        else
        {
            bus_ptr_->unsubscribe_generic(event_type_, id_);
        }
        bus_ptr_ = nullptr;
        id_ = 0;
    }
}

void cyanvne::platform::EventBus::unsubscribe_generic(std::type_index event_type, uint64_t subscription_id)
{
	std::lock_guard<std::mutex> lock(listeners_mutex_);

	auto it = generic_listeners_.find(event_type);

	if (it != generic_listeners_.end())
	{
		auto& callbacks = it->second;
		callbacks.erase(
			std::ranges::remove_if(callbacks,
			                       [subscription_id](const auto& wrapper_ptr) -> bool
			                       {
				                       return wrapper_ptr && wrapper_ptr->getSubscriptionID() == subscription_id;
			                       }).begin(),
			callbacks.end());

		if (callbacks.empty())
		{
			generic_listeners_.erase(it);
		}
	}
}

void cyanvne::platform::EventBus::unsubscribe_sdl(uint64_t subscription_id)
{
	std::lock_guard<std::mutex> lock(listeners_mutex_);

	sdl_listeners_.erase(
		std::ranges::remove_if(sdl_listeners_,
		                       [subscription_id](const auto& wrapper_ptr) -> bool
		                       {
			                       return wrapper_ptr && wrapper_ptr->getSubscriptionID() == subscription_id;
		                       }).begin(),
		sdl_listeners_.end());
}

cyanvne::platform::Subscription cyanvne::platform::EventBus::subscribeSDL(
	std::function<void(const SDL_Event&)> callback)
{
	if (!callback)
	{
		return {};
	}

	uint64_t id = next_subscription_id_++;
	auto wrapper = std::make_shared<SDLCallbackWrapper>(std::move(callback), id);

	std::lock_guard<std::mutex> lock(listeners_mutex_);

	sdl_listeners_.push_back(wrapper);
	return {this, std::type_index(typeid(SDL_Event)), id, true};
}

bool cyanvne::platform::EventBus::processAndPublishSDL(const SDL_Event& sdl_event, const ImGuiIO& imgui_io)
{
	bool event_for_game = true;

	switch (sdl_event.type)
	{
	case SDL_EVENT_MOUSE_MOTION:
	case SDL_EVENT_MOUSE_BUTTON_DOWN:
	case SDL_EVENT_MOUSE_BUTTON_UP:
	case SDL_EVENT_MOUSE_WHEEL:
		if (imgui_io.WantCaptureMouse)
			event_for_game = false;
		break;
	case SDL_EVENT_KEY_DOWN:
	case SDL_EVENT_KEY_UP:
		if (imgui_io.WantCaptureKeyboard)
			event_for_game = false;
		break;
	default: ;
	}

	if (imgui_io.WantTextInput &&
		(sdl_event.type == SDL_EVENT_KEY_DOWN || sdl_event.type == SDL_EVENT_KEY_UP || sdl_event.type == SDL_EVENT_TEXT_INPUT))
	{
		event_for_game = false;
	}

	if (!event_for_game)
	{
		return false;
	}

	SDL_Event event_copy = sdl_event;
	EventDispatchTask* task_ptr = new (std::nothrow) EventDispatchTask();

	if (!task_ptr)
	{
		return false;
	}

	task_ptr->dispatch_function = [this, event_data = event_copy]()
	{
		std::vector<std::shared_ptr<SDLCallbackWrapper>> sdl_callbacks_to_invoke;

		{
			std::lock_guard<std::mutex> lock(listeners_mutex_);
			sdl_callbacks_to_invoke = sdl_listeners_;
		}
		for (const auto& wrapper_ptr : sdl_callbacks_to_invoke)
		{
			if (wrapper_ptr)
			{
				wrapper_ptr->invoke(&event_data);
			}
		}
	};

	if (!event_queue_.push(task_ptr))
	{
		delete task_ptr;
		return false;
	}

	return true;
}

void cyanvne::platform::EventBus::dispatchPendingEvents()
{
	EventDispatchTask* task_ptr = nullptr;
	while (event_queue_.pop(task_ptr))
	{
		if (task_ptr)
		{
			if (task_ptr->dispatch_function)
			{
				task_ptr->dispatch_function();
			}
			delete task_ptr;
		}
	}
}
