//
// Created by unirz on 2025/8/13.
//

#ifndef CYANVNE_UNIFIEDCONCURRENCYMANAGER_H
#define CYANVNE_UNIFIEDCONCURRENCYMANAGER_H

#include <vector>
#include <thread>
#include <functional>
#include <optional>
#include <queue>
#include <mutex>
#include <condition_variable>
#include <boost/asio/experimental/awaitable_operators.hpp>
#include <memory>
#include <stdexcept>
#include <atomic>
#include <future>
#include <tuple>

#include <boost/asio.hpp>
#include "Core/Logger/Logger.h"

#if !defined(BOOST_ASIO_HAS_CO_AWAIT)
#error "C++20 Coroutine support is required. Please use a compatible compiler."
#endif

namespace cyanvne::platform::concurrency
{
    namespace asio = boost::asio;

    class UnifiedConcurrencyManager
    {
    private:
        template<typename T>
        class BlockingQueue
        {
        public:
            void push(T value)
            {
                {
                    std::lock_guard<std::mutex> lock(m_mutex);
                    m_queue.push(std::move(value));
                }
                m_cond.notify_one();
            }

            bool try_pop(T &value)
            {
                std::lock_guard<std::mutex> lock(m_mutex);
                if (m_queue.empty())
                {
                    return false;
                }
                value = std::move(m_queue.front());
                m_queue.pop();
                return true;
            }

            bool pop(T &value)
            {
                std::unique_lock<std::mutex> lock(m_mutex);
                m_cond.wait(lock, [this]
                { return !m_queue.empty() || m_stopped; });
                if (m_stopped && m_queue.empty())
                {
                    return false;
                }
                value = std::move(m_queue.front());
                m_queue.pop();
                return true;
            }

            void stop()
            {
                {
                    std::lock_guard<std::mutex> lock(m_mutex);
                    m_stopped = true;
                }
                m_cond.notify_all();
            }

        private:
            std::queue<T> m_queue;
            std::mutex m_mutex;
            std::condition_variable m_cond;
            bool m_stopped = false;
        };

    public:
        explicit UnifiedConcurrencyManager(size_t io_thread_count = 0, size_t worker_thread_count = 0)
                : m_io_thread_count(io_thread_count > 0 ? io_thread_count : std::thread::hardware_concurrency()),
                  m_worker_thread_count(
                          worker_thread_count > 0 ? worker_thread_count : std::thread::hardware_concurrency()),
                  m_io_context(static_cast<int>(m_io_thread_count)),
                  m_work_guard(asio::make_work_guard(m_io_context))
        {
            core::GlobalLogger::getCoreLogger()->info("[UCM] Initialized with {} IO threads and {} worker threads.",
                                                      m_io_thread_count, m_worker_thread_count);
            start();
        }

        ~UnifiedConcurrencyManager()
        {
            stop();
        }

        UnifiedConcurrencyManager(const UnifiedConcurrencyManager &) = delete;

        UnifiedConcurrencyManager &operator=(const UnifiedConcurrencyManager &) = delete;

        void start()
        {
            if (m_running)
                return;

            core::GlobalLogger::getCoreLogger()->info("[UCM] Starting IO thread pool...");
            for (size_t i = 0; i < m_io_thread_count; ++i)
            {
                m_io_threads.emplace_back([this]()
                                          { m_io_context.run(); });
            }

            core::GlobalLogger::getCoreLogger()->info("[UCM] Starting Worker thread pool...");
            for (size_t i = 0; i < m_worker_thread_count; ++i)
            {
                m_worker_threads.emplace_back([this]()
                                              {
                                                  while (true)
                                                  {
                                                      std::function < void() > task;
                                                      if (!m_worker_queue.pop(task))
                                                      {
                                                          break;
                                                      }
                                                      try
                                                      {
                                                          task();
                                                      } catch (const std::exception &e)
                                                      {
                                                          core::GlobalLogger::getCoreLogger()->error(
                                                                  "[Worker Thread] Exception caught: {}", e.what());
                                                      }
                                                  }
                                              });
            }
            m_running = true;
        }

        void stop()
        {
            if (!m_running)
                return;

            core::GlobalLogger::getCoreLogger()->info("[UCM] Stopping all thread pools...");

            m_worker_queue.stop();
            for (auto &t: m_worker_threads)
            {
                if (t.joinable())
                    t.join();
            }
            m_worker_threads.clear();

            m_main_thread_queue.stop();

            m_work_guard.reset();
            m_io_context.stop();
            for (auto &t: m_io_threads)
            {
                if (t.joinable())
                    t.join();
            }
            m_io_threads.clear();

            m_running = false;
            core::GlobalLogger::getCoreLogger()->info("[UCM] All thread pools stopped.");
        }

        void execute_main_thread_tasks()
        {
            std::function < void() > task;
            while (m_main_thread_queue.try_pop(task))
            {
                try
                {
                    task();
                }
                catch (const std::exception &e)
                {
                    core::GlobalLogger::getCoreLogger()->error("[Main Thread Task] Exception caught: {}", e.what());
                }
            }
        }

        void submit_main(std::function<void()> task)
        {
            m_main_thread_queue.push(std::move(task));
        }

        template<typename F>
        auto submit_main(F &&task) -> std::future<decltype(task())>
        {
            using ReturnType = decltype(task());
            auto promise = std::make_shared<std::promise<ReturnType>>();
            auto future = promise->get_future();

            m_main_thread_queue.push([promise, task = std::forward<F>(task)]() mutable
                                     {
                                         try
                                         {
                                             if constexpr (std::is_void_v<ReturnType>)
                                             {
                                                 task();
                                                 promise->set_value();
                                             } else
                                             {
                                                 promise->set_value(task());
                                             }
                                         } catch (...)
                                         {
                                             promise->set_exception(std::current_exception());
                                         }
                                     });

            return future;
        }

        template<typename F>
        auto submit_worker(F &&task) -> std::future<decltype(task())>
        {
            using ReturnType = decltype(task());

            auto promise = std::make_shared<std::promise<ReturnType>>();
            auto future = promise->get_future();

            m_worker_queue.push([promise, task = std::forward<F>(task)]() mutable
                                {
                                    try
                                    {
                                        if constexpr (std::is_void_v<ReturnType>)
                                        {
                                            task();
                                            promise->set_value();
                                        } else
                                        {
                                            promise->set_value(task());
                                        }
                                    } catch (...)
                                    {
                                        promise->set_exception(std::current_exception());
                                    }
                                });

            return future;
        }

        void submit_io(asio::awaitable<void> task)
        {
            if (!m_running)
                throw std::runtime_error("Manager is not running.");

            asio::co_spawn(m_io_context, std::move(task),
                           [logger = core::GlobalLogger::getCoreLogger()](std::exception_ptr e)
                           {
                               if (e)
                               {
                                   try
                                   {
                                       std::rethrow_exception(e);
                                   }
                                   catch (const std::exception &ex)
                                   {
                                       logger->error("[IO Coroutine] Unhandled exception: {}", ex.what());
                                   }
                               }
                           });
        }

        template<typename T>
        std::future<T> get_future_for_io(asio::awaitable<T> task)
        {
            auto promise = std::make_shared<std::promise<T>>();
            auto future = promise->get_future();

            auto wrapped_task = [promise, task = std::move(task)]() mutable -> asio::awaitable<void>
            {
                try
                {
                    if constexpr (std::is_void_v<T>)
                    {
                        co_await std::move(task);
                        promise->set_value();
                    } else
                    {
                        T result = co_await std::move(task);
                        promise->set_value(std::move(result));
                    }
                } catch (...)
                {
                    promise->set_exception(std::current_exception());
                }
            };

            asio::co_spawn(m_io_context, wrapped_task(), asio::detached);
            return future;
        }

        /**
         * @brief 提交后台任务，并在完成后将结果或异常传递到主线程执行后续操作。
         */
        template<typename WorkerFunc, typename MainThreadContinuation>
        void
        submit_worker_and_continue_on_main(WorkerFunc &&worker_task, MainThreadContinuation &&main_thread_continuation)
        {
            auto combined_task = [this,
                    worker = std::forward<WorkerFunc>(worker_task),
                    continuation = std::forward<MainThreadContinuation>(main_thread_continuation)]() mutable
            {
                using WorkerReturnType = decltype(worker());
                std::optional<WorkerReturnType> result;
                std::exception_ptr exception = nullptr;

                try
                {
                    if constexpr (!std::is_void_v<WorkerReturnType>)
                    {
                        result = worker();
                    } else
                    {
                        worker();
                    }
                } catch (...)
                {
                    exception = std::current_exception();
                }

                m_main_thread_queue.push(
                        [res = std::move(result), ex = exception, cont = std::move(continuation)]() mutable
                        {
                            cont(std::move(res), ex);
                        });
            };
            m_worker_queue.push(std::move(combined_task));
        }

        asio::io_context &get_io_context()
        { return m_io_context; }

        auto get_executor()
        { return m_io_context.get_executor(); }

    private:
        const size_t m_io_thread_count;
        const size_t m_worker_thread_count;

        asio::io_context m_io_context;
        std::optional<asio::executor_work_guard<asio::io_context::executor_type>> m_work_guard;
        std::vector<std::thread> m_io_threads;

        BlockingQueue<std::function<void()>> m_worker_queue;
        std::vector<std::thread> m_worker_threads;

        BlockingQueue<std::function<void()>> m_main_thread_queue;

        std::atomic<bool> m_running{false};
    };

    template<typename... Awaitables>
    inline auto when_all(Awaitables &&... awaitables)
    {
        using namespace asio::experimental::awaitable_operators;
        return (std::forward<Awaitables>(awaitables) && ...);
    }
}

#endif //CYANVNE_UNIFIEDCONCURRENCYMANAGER_H