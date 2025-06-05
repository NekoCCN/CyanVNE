#include "EventBus.h"

#include "backends/imgui_impl_sdl3.h"

namespace cyanvne
{
    namespace platform
    {

        Subscription::Subscription(Subscription&& other) noexcept
            : bus_ptr_(other.bus_ptr_),
            event_type_(other.event_type_),
            id_(other.id_),
            is_sdl_subscription_(other.is_sdl_subscription_),
            sdl_specific_type_filter_(other.sdl_specific_type_filter_)
        {
            other.bus_ptr_ = nullptr;
            other.id_ = 0;
            other.sdl_specific_type_filter_ = static_cast<SDL_EventType>(0);
        }

        Subscription& Subscription::operator=(Subscription&& other) noexcept
        {
            if (this != &other)
            {
                release();
                bus_ptr_ = other.bus_ptr_;
                event_type_ = other.event_type_;
                id_ = other.id_;
                is_sdl_subscription_ = other.is_sdl_subscription_;
                sdl_specific_type_filter_ = other.sdl_specific_type_filter_;
                other.bus_ptr_ = nullptr;
                other.id_ = 0;
                other.sdl_specific_type_filter_ = static_cast<SDL_EventType>(0);
            }
            return *this;
        }

        Subscription::~Subscription()
        {
            release();
        }

        void Subscription::release()
        {
            if (bus_ptr_ && id_ != 0)
            {
                if (is_sdl_subscription_)
                {
                    bus_ptr_->unsubscribe_sdl(id_, sdl_specific_type_filter_);
                }
                else
                {
                    bus_ptr_->unsubscribe_generic(event_type_, id_);
                }
                bus_ptr_ = nullptr;
                id_ = 0;
            }
        }

        bool Subscription::isValid() const
        {
            return bus_ptr_ != nullptr && id_ != 0;
        }
        uint64_t Subscription::getID() const
        {
            return id_;
        }
        bool Subscription::isSDLSubscription() const
        {
            return is_sdl_subscription_;
        }
        SDL_EventType Subscription::getSDLSpecificTypeFilter() const
        {
            return sdl_specific_type_filter_;
        }

        EventBus::~EventBus()
        {
            EventDispatchTask* task_ptr = nullptr;
            while (event_queue_.pop(task_ptr))
            {
                if (task_ptr)
                {
                    std::lock_guard<std::mutex> lock(task_pool_mutex_);
                    task_pool_.destroy(task_ptr);
                }
            }
        }

        void EventBus::unsubscribe_generic(std::type_index event_type, uint64_t subscription_id)
        {
            std::lock_guard<std::mutex> lock(listeners_mutex_);
            auto it = generic_listeners_.find(event_type);
            if (it != generic_listeners_.end())
            {
                auto& callbacks = it->second;
                auto [first, last] = std::ranges::remove_if(callbacks,
                    [subscription_id](const auto& wrapper_ptr) -> bool
                    {
                        return wrapper_ptr && wrapper_ptr->getSubscriptionID() == subscription_id;
                    });
                callbacks.erase(first, last);
                if (callbacks.empty())
                {
                    generic_listeners_.erase(it);
                }
            }
        }

        void EventBus::unsubscribe_sdl(uint64_t subscription_id, SDL_EventType specific_type_filter)
        {
            std::lock_guard<std::mutex> lock(listeners_mutex_);
            auto [first, last] = std::ranges::remove_if(sdl_listeners_,
                [subscription_id](const auto& wrapper_ptr) -> bool
                {
                    return wrapper_ptr && wrapper_ptr->getSubscriptionID() == subscription_id;
                });

            bool removed_from_main_list = false;
            if (first != sdl_listeners_.end())
            {
                if (std::distance(first, static_cast<std::vector<std::shared_ptr<ICallbackWrapper>>::iterator>(last)) > 0)
                {
                    removed_from_main_list = true;
                }
            }
            if (first != last)
            {
                sdl_listeners_.erase(first, last);
                removed_from_main_list = true;
            }

            if (removed_from_main_list)
            {
                if (specific_type_filter != static_cast<SDL_EventType>(0))
                {
                    auto it_count = sdl_event_type_subscriber_counts_.find(specific_type_filter);
                    if (it_count != sdl_event_type_subscriber_counts_.end())
                    {
                        if (--(it_count->second) == 0)
                        {
                            sdl_event_type_subscriber_counts_.erase(it_count);
                        }
                    }
                    else
                    {
                        core::GlobalLogger::getCoreLogger()->warn("EventBus: Count decrement for untracked SDL type {}.", static_cast<unsigned int>(specific_type_filter));
                    }
                }
                else
                {
                    if (sdl_wildcard_subscriber_count_.load(std::memory_order_relaxed) > 0)
                    {
                        --sdl_wildcard_subscriber_count_;
                    }
                    else {
                        core::GlobalLogger::getCoreLogger()->warn("EventBus: Wildcard count already zero before decrement.");
                    }
                }
            }
        }

        Subscription EventBus::subscribeSDL(std::function<bool(const SDL_Event&)> callback)
        {
            return subscribeSDL(static_cast<SDL_EventType>(0), std::move(callback));
        }

        Subscription EventBus::subscribeSDL(SDL_EventType specific_event_type, std::function<bool(const SDL_Event&)> callback)
        {
            if (!callback)
            {
                return {};
            }
            uint64_t id = next_subscription_id_++;
            auto wrapper = std::make_shared<SDLCallbackWrapper>(std::move(callback), id, specific_event_type);
            std::lock_guard<std::mutex> lock(listeners_mutex_);
            sdl_listeners_.push_back(wrapper);

            if (specific_event_type != static_cast<SDL_EventType>(0))
            {
                ++sdl_event_type_subscriber_counts_[specific_event_type];
            }
            else
            {
                ++sdl_wildcard_subscriber_count_;
            }
            return { this, std::type_index(typeid(SDL_Event)), id, true, specific_event_type };
        }

        bool EventBus::processAndPublishSDL(const SDL_Event& sdl_event, const ImGuiIO& imgui_io)
        {
            bool event_for_game = true;
            switch (sdl_event.type)
            {
            case SDL_EVENT_MOUSE_MOTION:
            case SDL_EVENT_MOUSE_BUTTON_DOWN:
            case SDL_EVENT_MOUSE_BUTTON_UP:
            case SDL_EVENT_MOUSE_WHEEL:
                if (imgui_io.WantCaptureMouse)
                {
                    event_for_game = false;
                }
                break;
            case SDL_EVENT_KEY_DOWN:
            case SDL_EVENT_KEY_UP:
                if (imgui_io.WantCaptureKeyboard)
                {
                    event_for_game = false;
                }
                break;
            default:
                break;
            }
            if (imgui_io.WantTextInput &&
                (sdl_event.type == SDL_EVENT_KEY_DOWN || sdl_event.type == SDL_EVENT_KEY_UP || sdl_event.type == SDL_EVENT_TEXT_INPUT))
            {
                event_for_game = false;
            }

            ImGui_ImplSDL3_ProcessEvent(&sdl_event);

            if (!event_for_game)
            {
                return false;
            }

            bool has_specific_subscriber = false;
            bool has_wildcard_subscriber = (sdl_wildcard_subscriber_count_.load(std::memory_order_relaxed) > 0);

            auto it = sdl_event_type_subscriber_counts_.find(static_cast<SDL_EventType>(sdl_event.type));
            if (it != sdl_event_type_subscriber_counts_.end())
            {
                if (it->second.load(std::memory_order_relaxed) > 0)
                {
                    has_specific_subscriber = true;
                }
            }

            if (!has_specific_subscriber && !has_wildcard_subscriber)
            {
                return false;
            }

            EventDispatchTask* task_ptr;
            {
                std::lock_guard<std::mutex> pool_lock(task_pool_mutex_);
                task_ptr = task_pool_.construct();
            }
            if (!task_ptr)
            {
                core::GlobalLogger::getCoreLogger()->error("EventBus: Failed to construct Task from pool. SDL Event Dropped.");
                return false;
            }

            SDL_Event event_copy = sdl_event;
            task_ptr->dispatch_function = [this, event_data = event_copy]()
                {
                    std::vector<std::shared_ptr<ICallbackWrapper>> callbacks_to_invoke_sdl;
                    {
                        std::lock_guard<std::mutex> lock(listeners_mutex_);
                        callbacks_to_invoke_sdl = sdl_listeners_;
                    }
                    for (const auto& wrapper_ptr : callbacks_to_invoke_sdl)
                    {
                        if (wrapper_ptr)
                        {
                            if (wrapper_ptr->invoke(&event_data))
                            {
                                break;
                            }
                        }
                    }
                };
            if (!event_queue_.push(task_ptr))
            {
                core::GlobalLogger::getCoreLogger()->error("EventBus: Queue full. SDL Event Dropped.");
                std::lock_guard<std::mutex> pool_lock(task_pool_mutex_);
                task_pool_.destroy(task_ptr);
                return false;
            }
            return true;
        }

        void EventBus::dispatchPendingEvents()
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
                    std::lock_guard<std::mutex> lock(task_pool_mutex_);
                    task_pool_.destroy(task_ptr);
                }
            }
        }

        size_t EventBus::getSDLSubscriberCount(SDL_EventType specific_event_type) const
        {
            std::lock_guard<std::mutex> lock(listeners_mutex_);
            if (specific_event_type == static_cast<SDL_EventType>(0))
            {
                return sdl_wildcard_subscriber_count_.load(std::memory_order_relaxed);
            }
            auto it = sdl_event_type_subscriber_counts_.find(specific_event_type);
            return (it != sdl_event_type_subscriber_counts_.end()) ? it->second.load(std::memory_order_relaxed) : 0;
        }

        size_t EventBus::getTotalSubscriberCount() const
        {
            size_t total = 0;
            std::lock_guard<std::mutex> lock(listeners_mutex_);
            for (const auto& val : generic_listeners_ | std::views::values)
            {
                total += val.size();
            }
            total += sdl_listeners_.size();
            return total;
        }
    }
}
