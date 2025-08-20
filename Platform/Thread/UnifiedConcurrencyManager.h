//
// Created by unirz on 2025/8/13.
//

#ifndef CYANVNE_UNIFIEDCONCURRENCYMANAGER_H
#define CYANVNE_UNIFIEDCONCURRENCYMANAGER_H

#include <boost/asio.hpp>
#include <boost/fiber/all.hpp>
#include <boost/version.hpp>
#include <iostream>
#include <map>
#include <memory>
#include <string>
#include <string_view>
#include <thread>
#include <vector>
#include <stdexcept>
#include <functional>
#include <optional>

namespace cyanvne::platform::concurrency
{
    namespace asio = boost::asio;
    namespace fibers = boost::fibers;

    class IoContextRunner
    {
    public:
        IoContextRunner(const IoContextRunner &) = delete;

        IoContextRunner &operator=(const IoContextRunner &) = delete;

        explicit IoContextRunner(size_t thread_count = 1)
                : work_guard_(asio::make_work_guard(ioc_))
        {
            if (thread_count == 0)
                thread_count = 1;

            threads_.reserve(thread_count);
            for (size_t i = 0; i < thread_count; ++i)
            {
                threads_.emplace_back([this]()
                {
                    try
                    {
                        ioc_.run();
                    } catch (const std::exception &e) {
                        std::cerr << "IoContextRunner thread caught exception: " << e.what() << std::endl;
                    }
                });
            }
        }

        ~IoContextRunner()
        {
            stop();
        }

        asio::io_context &getContext()
        {
            return ioc_;
        }

        void stop()
        {
            if (!ioc_.stopped())
            {
                work_guard_.reset();
                ioc_.stop();
                for (auto &t: threads_)
                {
                    if (t.joinable())
                    {
                        t.join();
                    }
                }
            }
        }

    private:
        asio::io_context ioc_;
        using work_guard_type = asio::executor_work_guard<asio::io_context::executor_type>;
        work_guard_type work_guard_;
        std::vector<std::thread> threads_;
    };

    class StacklessCoroutineEngine
    {
    public:
        explicit StacklessCoroutineEngine(const std::map<std::string, size_t> &config)
        {
            if (config.find("default") == config.end())
            {
                throw std::invalid_argument("Stackless Engine: Config must include a 'default' context.");
            }
            for (const auto &[name, thread_count]: config)
            {
                runners_[name] = std::make_unique<IoContextRunner>(thread_count);
            }
        }

        void shutdown()
        {
            for (auto &[name, runner]: runners_)
            {
                if (runner) runner->stop();
            }
            runners_.clear();
        }

        template<typename Awaitable>
        void spawn(std::string_view context_name, Awaitable &&awaitable)
        {
            asio::co_spawn(get_runner(context_name).getContext(), std::forward<Awaitable>(awaitable), asio::detached);
        }

    private:
        IoContextRunner &get_runner(std::string_view name)
        {
            auto it = runners_.find(std::string(name));
            if (it == runners_.end())
            {
                throw std::runtime_error("Stackless Engine: Context '" + std::string(name) + "' not found.");
            }
            return *it->second;
        }

        std::map<std::string, std::unique_ptr<IoContextRunner>> runners_;
    };

    class FiberEngine {
    public:
        explicit FiberEngine(size_t thread_count = 1) {
            if (thread_count == 0) thread_count = 1;
            scheduler_ = std::make_unique<fibers::asio::round_robin>(ioc_);
            work_guard_ = std::make_unique<asio::executor_work_guard<asio::io_context::executor_type>>(
                    ioc_.get_executor());

            threads_.reserve(thread_count);
            for (size_t i = 0; i < thread_count; ++i) {
                threads_.emplace_back([this, sched_ptr = scheduler_.get()]() {
                    fibers::use_scheduling_algorithm(sched_ptr);
                    ioc_.run();
                });
            }
        }

        void shutdown() {
            if (!ioc_.stopped()) {
                work_guard_.reset();
                ioc_.stop();
                for (auto &t: threads_) {
                    if (t.joinable()) t.join();
                }
            }
        }

        template<typename Fn>
        void spawn(Fn &&fn) {
            asio::post(ioc_, [fn = std::forward<Fn>(fn)]() mutable {
                fibers::fiber(std::move(fn)).detach();
            });
        }

    private:
        asio::io_context ioc_;
        std::vector<std::thread> threads_;
        std::unique_ptr<asio::executor_work_guard<asio::io_context::executor_type>> work_guard_;
        std::unique_ptr<fibers::sched_algorithm> scheduler_;
    };

    struct StacklessConfig {
        std::map<std::string, size_t> contexts;
    };

    struct FiberConfig {
        size_t thread_count = 0;
    };

    struct UnifiedConfig {
        std::optional<StacklessConfig> stackless_config;
        std::optional<FiberConfig> fiber_config;
    };

    class UnifiedConcurrencyManager {
    public:
        UnifiedConcurrencyManager(const UnifiedConcurrencyManager &) = delete;

        UnifiedConcurrencyManager &operator=(const UnifiedConcurrencyManager &) = delete;

        static UnifiedConcurrencyManager &get() {
            static UnifiedConcurrencyManager instance;
            return instance;
        }

        static void initialize(const UnifiedConfig &config) {
            get()._initialize(config);
        }

        static void shutdown() {
            get()._shutdown();
        }

        template<typename Awaitable>
        void co_spawn(std::string_view context_name, Awaitable &&awaitable) {
            if (!stackless_engine_) {
                throw std::runtime_error("Stackless Coroutine Engine is not initialized.");
            }
            stackless_engine_->spawn(context_name, std::forward<Awaitable>(awaitable));
        }

        template<typename Awaitable>
        void co_spawn(Awaitable &&awaitable) {
            co_spawn("default", std::forward<Awaitable>(awaitable));
        }

        template<typename Fn>
        void fiber_spawn(Fn &&fn) {
            if (!fiber_engine_) {
                throw std::runtime_error("Fiber Engine is not initialized.");
            }
            fiber_engine_->spawn(std::forward<Fn>(fn));
        }

    private:
        UnifiedConcurrencyManager() = default;

        ~UnifiedConcurrencyManager() { _shutdown(); }

        void _initialize(const UnifiedConfig &config) {
            if (config.stackless_config) {
                stackless_engine_ = std::make_unique<StacklessCoroutineEngine>(config.stackless_config.value());
            }
            if (config.fiber_config) {
                fiber_engine_ = std::make_unique<FiberEngine>(config.fiber_config.value().thread_count);
            }
        }

        void _shutdown() {
            if (fiber_engine_) fiber_engine_->shutdown();
            if (stackless_engine_) stackless_engine_->shutdown();
        }

        std::unique_ptr<StacklessCoroutineEngine> stackless_engine_;
        std::unique_ptr<FiberEngine> fiber_engine_;
    };
}

#endif //CYANVNE_UNIFIEDCONCURRENCYMANAGER_H
