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
#include <ranges>
#include <SDL3/SDL.h>
#include "imgui.h"
#include <boost/lockfree/queue.hpp>
#include <boost/pool/object_pool.hpp>


namespace cyanvne
{
    namespace platform
    {
        class EventBus;

        class Subscription
        {
        private:
            EventBus* bus_ptr_ = nullptr;
            std::type_index event_type_;
            uint64_t id_ = 0;
            bool is_sdl_subscription_ = false;
            SDL_EventType sdl_specific_type_filter_ = static_cast<SDL_EventType>(0);

            Subscription(EventBus* bus, std::type_index event_type, uint64_t id,
                bool is_sdl = false, SDL_EventType sdl_filter = static_cast<SDL_EventType>(0))
                : bus_ptr_(bus), event_type_(event_type), id_(id), is_sdl_subscription_(is_sdl), sdl_specific_type_filter_(sdl_filter)
            {  }
            friend class EventBus;

        public:
            Subscription() : event_type_(typeid(void))
            {  }
            Subscription(Subscription&& other) noexcept;
            Subscription& operator=(Subscription&& other) noexcept;
            Subscription(const Subscription&) = delete;
            Subscription& operator=(const Subscription&) = delete;
            ~Subscription();
            void release();
            bool isValid() const;
            uint64_t getID() const;
            bool isSDLSubscription() const;
            SDL_EventType getSDLSpecificTypeFilter() const;
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
                virtual bool invoke(const void* event_data) = 0;
                virtual uint64_t getSubscriptionID() const = 0;
                virtual SDL_EventType getSDLFilterType() const { return static_cast<SDL_EventType>(0); }
                virtual ~ICallbackWrapper() = default;
            };

            template<typename EventT>
            struct GenericCallbackWrapper : ICallbackWrapper
            {
                std::function<bool(const EventT&)> callback_;
                uint64_t subscription_id_;
                GenericCallbackWrapper(std::function<bool(const EventT&)> cb, uint64_t id)
                    : callback_(std::move(cb)), subscription_id_(id)
                {  }
                bool invoke(const void* event_data) override
                {
                    if (callback_)
                    {
                        try
                        {
                            return callback_(*static_cast<const EventT*>(event_data));
                        }
                        catch (const std::exception& e)
                        {
                            core::GlobalLogger::getCoreLogger()->error("EventBus Exc (Generic): {:s}", e.what());
                        }
                        catch (...)
                        {
                            core::GlobalLogger::getCoreLogger()->error("EventBus Exc (Generic): Unknown");
                        }
                    }
                    return false;
                }
                uint64_t getSubscriptionID() const override
                {
                    return subscription_id_;
                }
            };

            struct SDLCallbackWrapper : ICallbackWrapper
            {
                std::function<bool(const SDL_Event&)> callback_;
                uint64_t subscription_id_;
                SDL_EventType filter_event_type_;

                SDLCallbackWrapper(std::function<bool(const SDL_Event&)> cb, uint64_t id, SDL_EventType filter)
                    : callback_(std::move(cb)), subscription_id_(id), filter_event_type_(filter)
                {  }
                bool invoke(const void* event_data) override
                {
                    if (callback_)
                    {
                        const SDL_Event* sdl_event = static_cast<const SDL_Event*>(event_data);
                        if (filter_event_type_ == static_cast<SDL_EventType>(0) || sdl_event->type == filter_event_type_)
                        {
                            try
                            {
                                return callback_(*sdl_event);
                            }
                            catch (const std::exception& e)
                            {
                                core::GlobalLogger::getCoreLogger()->error("EventBus Exc (SDL): {:s}", e.what());
                            }
                            catch (...)
                            {
                                core::GlobalLogger::getCoreLogger()->error("EventBus Exc (SDL): Unknown");
                            }
                        }
                    }
                    return false;
                }
                uint64_t getSubscriptionID() const override
                {
                    return subscription_id_;
                }
                SDL_EventType getSDLFilterType() const override
                {
                    return filter_event_type_;
                }
            };

            std::map<std::type_index, std::vector<std::shared_ptr<ICallbackWrapper>>> generic_listeners_;
            std::vector<std::shared_ptr<ICallbackWrapper>> sdl_listeners_;

            mutable std::mutex listeners_mutex_;
            std::atomic<uint64_t> next_subscription_id_{ 1 };
            std::map<SDL_EventType, std::atomic<int>> sdl_event_type_subscriber_counts_;
            std::atomic<int> sdl_wildcard_subscriber_count_{ 0 };

            struct EventDispatchTask
            {
                std::function<void()> dispatch_function;
                EventDispatchTask() = default;
            };

            boost::object_pool<EventDispatchTask> task_pool_;
            mutable std::mutex task_pool_mutex_;

            boost::lockfree::queue<EventDispatchTask*, boost::lockfree::capacity<1546>> event_queue_;

            void unsubscribe_generic(std::type_index event_type, uint64_t subscription_id);
            void unsubscribe_sdl(uint64_t subscription_id, SDL_EventType specific_type_filter);

            friend class Subscription;

        public:
            EventBus() = default;
            EventBus(const EventBus&) = delete;
            EventBus& operator=(const EventBus&) = delete;
            EventBus(EventBus&&) = delete;
            EventBus& operator=(EventBus&&) = delete;
            ~EventBus();

            template<typename EventT>
            Subscription subscribe(std::function<bool(const EventT&)> callback)
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

                return { this, type_idx, id, false, static_cast<SDL_EventType>(0) };
            }

            Subscription subscribeSDL(std::function<bool(const SDL_Event&)> callback);
            Subscription subscribeSDL(SDL_EventType specific_event_type, std::function<bool(const SDL_Event&)> callback);

            template<typename EventT>
            void publish(const EventT& event)
            {
                static_assert(!std::is_same_v<EventT, SDL_Event>, "For SDL_Event, use processAndPublishSDL().");
                std::type_index type_idx(typeid(EventT));
                {
                    std::lock_guard<std::mutex> lock(listeners_mutex_);
                    auto it = generic_listeners_.find(type_idx);
                    if (it == generic_listeners_.end() || it->second.empty())
                    {
                        return;
                    }
                }
                EventDispatchTask* task_ptr;
                {
                    std::lock_guard<std::mutex> pool_lock(task_pool_mutex_);
                    task_ptr = task_pool_.construct();
                }
                if (!task_ptr)
                {
                    core::GlobalLogger::getCoreLogger()->error("EventBus: Failed to construct Task from pool. Generic Event Dropped.");
                    return;
                }
                EventT event_copy = event;
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
                                if (wrapper_ptr->invoke(&event_data))
                                {
                                    break;
                                }
                            }
                        }
                    };
                if (!event_queue_.push(task_ptr))
                {
                    core::GlobalLogger::getCoreLogger()->error("EventBus: Queue full. Generic Event Dropped.");
                    std::lock_guard<std::mutex> pool_lock(task_pool_mutex_);
                    task_pool_.destroy(task_ptr);
                }
            }

            template<typename EventT>
            void publish(EventT&& event)
            {
                static_assert(!std::is_same_v<EventT, SDL_Event>, "For SDL_Event, use processAndPublishSDL().");
                std::type_index type_idx(typeid(EventT));
                {
                    std::lock_guard<std::mutex> lock(listeners_mutex_);
                    auto it = generic_listeners_.find(type_idx);
                    if (it == generic_listeners_.end() || it->second.empty())
                    {
                        return;
                    }
                }
                EventDispatchTask* task_ptr;
                {
                    std::lock_guard<std::mutex> pool_lock(task_pool_mutex_);
                    task_ptr = task_pool_.construct();
                }
                if (!task_ptr)
                {
                    core::GlobalLogger::getCoreLogger()->error("EventBus: Failed to construct Task from pool. Generic Event (move) Dropped.");
                    return;
                }
                EventT event_moved = std::forward<EventT>(event);
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
                                if (wrapper_ptr->invoke(&event_data))
                                {
                                    break;
                                }
                            }
                        }
                    };
                if (!event_queue_.push(task_ptr))
                {
                    core::GlobalLogger::getCoreLogger()->error("EventBus: Queue full. Generic Event (move) Dropped.");
                    std::lock_guard<std::mutex> pool_lock(task_pool_mutex_);
                    task_pool_.destroy(task_ptr);
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

            size_t getSDLSubscriberCount(SDL_EventType specific_event_type = static_cast<SDL_EventType>(0)) const;
            size_t getTotalSubscriberCount() const;
        };
    }
}