#pragma once
#include <Core/Logger/Logger.h>
#include <functional>
#include <map>
#include <vector>
#include <memory>
#include <mutex>
#include <typeindex>
#include <algorithm>
#include <atomic>
#include <utility> 
#include <new>
#include <ranges>
#include <SDL3/SDL.h> 
#include "imgui.h"    
#include <boost/lockfree/queue.hpp>

namespace cyanvne
{
	namespace platform
	{
        class EventBus;

        class Subscription
        {
        private:
            EventBus* bus_ptr_ = nullptr;

            // RTTI
            std::type_index event_type_;

            uint64_t id_ = 0;
            bool is_sdl_subscription_ = false;

            Subscription(EventBus* bus, std::type_index event_type, uint64_t id, bool is_sdl = false)
                : bus_ptr_(bus), event_type_(event_type), id_(id), is_sdl_subscription_(is_sdl)
            {  }

            friend class EventBus;
        public:
            Subscription() : event_type_(typeid(void))
            {  }

            Subscription(Subscription&& other) noexcept
                : bus_ptr_(other.bus_ptr_),
                event_type_(other.event_type_),
                id_(other.id_),
                is_sdl_subscription_(other.is_sdl_subscription_)
            {
                other.bus_ptr_ = nullptr;
                other.id_ = 0;
            }

            Subscription& operator=(Subscription&& other) noexcept
            {
                if (this != &other)
                {
                    release();
                    bus_ptr_ = other.bus_ptr_;
                    event_type_ = other.event_type_;
                    id_ = other.id_;
                    is_sdl_subscription_ = other.is_sdl_subscription_;
                    other.bus_ptr_ = nullptr;
                    other.id_ = 0;
                }
                return *this;
            }

            Subscription(const Subscription&) = delete;
            Subscription& operator=(const Subscription&) = delete;


            void release();

            bool isValid() const
            {
                return bus_ptr_ != nullptr && id_ != 0;
            }

            uint64_t getID() const
            {
                return id_;
            }

            bool isSDLSubscription() const
            {
                return is_sdl_subscription_;
            }

            ~Subscription()
            {
                release();
            }
        };


        class EventBus
        {
        private:
            class ICallbackWrapper
            {
            protected:
                ICallbackWrapper() = default;
            public:
                ICallbackWrapper(ICallbackWrapper&&) = delete;
                ICallbackWrapper& operator=(ICallbackWrapper&&) = delete;
                ICallbackWrapper(const ICallbackWrapper&) = delete;
                ICallbackWrapper& operator=(const ICallbackWrapper&) = delete;

                virtual void invoke(const void* event_data) = 0;
                virtual uint64_t getSubscriptionID() const = 0;

                virtual ~ICallbackWrapper() = default;
            };

            template<typename EventT>
            struct GenericCallbackWrapper : ICallbackWrapper
            {
                std::function<void(const EventT&)> callback_;
                uint64_t subscription_id_;

                GenericCallbackWrapper(std::function<void(const EventT&)> cb, uint64_t id)
                    : callback_(std::move(cb)), subscription_id_(id)
                {  }

                void invoke(const void* event_data) override
                {
                    if (callback_)
                    {
                        callback_(*static_cast<const EventT*>(event_data));
                    }
                }

                uint64_t getSubscriptionID() const override
                {
                    return subscription_id_;
                }
            };

            struct SDLCallbackWrapper : ICallbackWrapper
            {
                std::function<void(const SDL_Event&)> callback_;
                uint64_t subscription_id_;

                SDLCallbackWrapper(std::function<void(const SDL_Event&)> cb, uint64_t id)
                    : callback_(std::move(cb)), subscription_id_(id)
                {  }

                void invoke(const void* event_data) override
                {
                    if (callback_)
                    {
                        try
                        {
                            callback_(*static_cast<const SDL_Event*>(event_data));
                        }
                        catch (const std::exception& e)
                        {
                            core::GlobalLogger::getCoreLogger()->error("Event Bus Excption Ignore : {:s}", e.what());
                        }
                        catch (...)
                        {
                            core::GlobalLogger::getCoreLogger()->error("Event Bus Excption Ignore : Unknown");
                        }
                    }
                }

                uint64_t getSubscriptionID() const override
                {
                    return subscription_id_;
                }
            };

            std::map<std::type_index, std::vector<std::shared_ptr<ICallbackWrapper>>> generic_listeners_;
            std::vector<std::shared_ptr<SDLCallbackWrapper>> sdl_listeners_;

            mutable std::mutex listeners_mutex_;
            std::atomic<uint64_t> next_subscription_id_ { 1 };

            struct EventDispatchTask
            {
                std::function<void()> dispatch_function;
            };

            boost::lockfree::queue<EventDispatchTask*, boost::lockfree::capacity<1024>> event_queue_;

            void unsubscribe_generic(std::type_index event_type, uint64_t subscription_id);
            void unsubscribe_sdl(uint64_t subscription_id);

            friend class Subscription;

        public:
            EventBus() = default;
            EventBus(const EventBus&) = delete;
            EventBus& operator=(const EventBus&) = delete;
            EventBus(EventBus&&) = delete;
            EventBus& operator=(EventBus&&) = delete;

            ~EventBus()
            {
                EventDispatchTask* task_ptr = nullptr;

                while (event_queue_.pop(task_ptr))
                {
                    delete task_ptr;
                }
            }

            template<typename EventT>
            Subscription subscribe(std::function<void(const EventT&)> callback)
            {
                static_assert(!std::is_same_v<EventT, SDL_Event>, "For SDL_Event, use subscribeSDL().");
                if (!callback)
                {
                    return {};
                }
                uint64_t id = next_subscription_id_++;

                auto wrapper = std::make_shared<GenericCallbackWrapper<EventT>>(std::move(callback), id);
                std::type_index type_idx(typeid(EventT));

                std::lock_guard<std::mutex> lock(listeners_mutex_);
                generic_listeners_[type_idx].push_back(wrapper);

                return {this, type_idx, id, false};
            }

            Subscription subscribeSDL(std::function<void(const SDL_Event&)> callback);

            template<typename EventT>
            void publish(const EventT& event)
            {
                static_assert(!std::is_same_v<EventT, SDL_Event>, "For SDL_Event, use processAndPublishSDL().");

                EventT event_copy = event;
                std::type_index type_idx(typeid(EventT));
                EventDispatchTask* task_ptr = new (std::nothrow) EventDispatchTask();

                if (!task_ptr)
                {
                    return;
                }
                task_ptr->dispatch_function = [this, type_idx, event_data = std::move(event_copy)]()
                    {
                        std::vector<std::shared_ptr<ICallbackWrapper>> callbacks_to_invoke;

                        {
                            std::lock_guard<std::mutex> lock(listeners_mutex_);
                            auto it = generic_listeners_.find(type_idx);
                            if (it != generic_listeners_.end())
                            {
                                callbacks_to_invoke = it->second;
                            }
                        }

                        for (const auto& wrapper_ptr : callbacks_to_invoke)
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
                }
            }

            template<typename EventT>
            void publish(EventT&& event)
            {
                static_assert(!std::is_same_v<EventT, SDL_Event>, "For SDL_Event, use processAndPublishSDL().");

                EventT event_moved = std::forward<EventT>(event);
                std::type_index type_idx(typeid(EventT));
                EventDispatchTask* task_ptr = new (std::nothrow) EventDispatchTask();

                if (!task_ptr)
                {
                    return;
                }

                task_ptr->dispatch_function = [this, type_idx, event_data = std::move(event_moved)]() mutable
                    {
                        std::vector<std::shared_ptr<ICallbackWrapper>> callbacks_to_invoke;

                        {
                            std::lock_guard<std::mutex> lock(listeners_mutex_);
                            auto it = generic_listeners_.find(type_idx);
                            if (it != generic_listeners_.end())
                            {
                                callbacks_to_invoke = it->second;
                            }
                        }

                        for (const auto& wrapper_ptr : callbacks_to_invoke)
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
                }
            }

            bool processAndPublishSDL(const SDL_Event& sdl_event, const ImGuiIO& imgui_io);
            void dispatchPendingEvents();

            template<typename EventT>
            size_t getSubscriberCount() const
            {
                static_assert(!std::is_same_v<EventT, SDL_Event>, "Use getSDLSubscriberCount() for SDL_Event.");
                std::type_index type_idx(typeid(EventT));
                
                std::lock_guard<std::mutex> lock(listeners_mutex_);
                
                auto it = generic_listeners_.find(type_idx);
                return (it != generic_listeners_.end()) ? it->second.size() : 0;
            }

            size_t getSDLSubscriberCount() const
            {
                std::lock_guard<std::mutex> lock(listeners_mutex_);
                return sdl_listeners_.size();
            }

            size_t getTotalSubscriberCount() const
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
        };
	}
}