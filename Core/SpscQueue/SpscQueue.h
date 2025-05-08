#pragma once
#include <atomic>
#include <cstddef>
#include <thread>
#include <chrono>

namespace vn
{
    namespace core
    {
        namespace spscqueue
        {
            class WaitStrategy
            {
            public:
                virtual ~WaitStrategy() = default;
                virtual void wait() = 0;
                virtual void reset() = 0;
            };

            class SpinWaitStrategy : public WaitStrategy
            {
            public:
                void wait() override { }
                void reset() override { }
            };

            class YieldWaitStrategy : public WaitStrategy
            {
            public:
                void wait() override
                {
                    std::this_thread::yield();
                }
                void reset() override { }
            };

            class BackoffWaitStrategy : public WaitStrategy
            {
            private:
                static constexpr int MIN_DELAY_US = 1;
                static constexpr int MAX_DELAY_US = 1000;
                int current_delay_us_ = MIN_DELAY_US;

            public:
                void wait() override
                {
                    std::this_thread::sleep_for(std::chrono::microseconds(current_delay_us_));
                    current_delay_us_ = std::min(current_delay_us_ * 2, MAX_DELAY_US);
                }

                void reset() override
                {
                    current_delay_us_ = MIN_DELAY_US;
                }
            };

            template <typename Tn> class SpscQueue
            {
            private:
                struct Node
                {
                    Tn data;
                    Node* next;
                    Node(const Tn& item) : data(item), next(nullptr) { }
                };

                static constexpr size_t CACHE_LINE_SIZE = 64;
                alignas(CACHE_LINE_SIZE) std::atomic<Node*> head_;
                alignas(CACHE_LINE_SIZE) std::atomic<Node*> tail_;
                Node* first_;
                size_t capacity_;

                std::unique_ptr<WaitStrategy> producer_wait_strategy_;
                std::unique_ptr<WaitStrategy> consumer_wait_strategy_;
            public:
                enum class WaitType
                {
                    Spin,
                    Yield,
                    Backoff
                };

                explicit SpscQueue(size_t capacity,
                    WaitType producer_wait = WaitType::Spin, WaitType consumer_wait = WaitType::Spin)
                    : capacity_(capacity)
                {
                    first_ = new Node(Tn{});
                    Node* current = first_;

                    for (size_t i = 1; i < capacity_; ++i)
                    {
                        current->next = new Node(Tn{});
                        current = current->next;
                    }

                    current->next = first_;
                    head_.store(first_, std::memory_order_relaxed);
                    tail_.store(first_, std::memory_order_relaxed);

                    setProducerWaitStrategy(producer_wait);
                    setConsumerWaitStrategy(consumer_wait);
                }
                SpscQueue(const SpscQueue&) = delete;
                SpscQueue& operator=(const SpscQueue&) = delete;

                ~SpscQueue()
                {
                    Node* current = first_;
                    for (size_t i = 0; i < capacity_; ++i)
                    {
                        Node* next = current->next;
                        delete current;
                        current = next;
                    }
                }

                void setProducerWaitStrategy(WaitType type)
                {
                    switch (type)
                    {
                    case WaitType::Spin:
                        producer_wait_strategy_ = std::make_unique<SpinWaitStrategy>();
                        break;
                    case WaitType::Yield:
                        producer_wait_strategy_ = std::make_unique<YieldWaitStrategy>();
                        break;
                    case WaitType::Backoff:
                        producer_wait_strategy_ = std::make_unique<BackoffWaitStrategy>();
                        break;
                    }
                }

                void setConsumerWaitStrategy(WaitType type)
                {
                    switch (type)
                    {
                    case WaitType::Spin:
                        consumer_wait_strategy_ = std::make_unique<SpinWaitStrategy>();
                        break;
                    case WaitType::Yield:
                        consumer_wait_strategy_ = std::make_unique<YieldWaitStrategy>();
                        break;
                    case WaitType::Backoff:
                        consumer_wait_strategy_ = std::make_unique<BackoffWaitStrategy>();
                        break;
                    }
                }

                bool enqueue(const Tn& item, bool retry = true)
                {
                    while (true)
                    {
                        Node* tail = tail_.load(std::memory_order_relaxed);
                        Node* next = tail->next;

                        if (next == head_.load(std::memory_order_acquire))
                        {
                            if (!retry) return false;
                            producer_wait_strategy_->wait();
                            continue;
                        }

                        next->data = item;
                        tail_.store(next, std::memory_order_release);
                        producer_wait_strategy_->reset();
                        return true;
                    }
                }


                bool dequeue(Tn& item, bool retry = true)
                {
                    while (true)
                    {
                        Node* head = head_.load(std::memory_order_relaxed);

                        if (head == tail_.load(std::memory_order_acquire))
                        {
                            if (!retry) return false;
                            consumer_wait_strategy_->wait();
                            continue;
                        }

                        item = head->next->data;
                        head_.store(head->next, std::memory_order_release);
                        consumer_wait_strategy_->reset();
                        return true;
                    }
                }

                bool empty() const
                {
                    return head_.load(std::memory_order_acquire) == tail_.load(std::memory_order_acquire);
                }
            };
        }
    }
}
