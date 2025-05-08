#pragma once
#ifndef VISUALNOVEL_TSVECTOR_H
#define VISUALNOVEL_TSVECTOR_H
#include <atomic>
#include <memory>
#include <stdexcept>
#include <mutex>
#include <shared_mutex>
#include <algorithm>

namespace vn
{
    namespace core
    {
        template <typename T> class ThreadSafeVector
        {
        private:
            alignas(64) std::atomic<size_t> current_size{ 0 };
            alignas(64) std::atomic<size_t> capacity{ 0 };

            std::atomic<std::unique_ptr<T[]>> data{ nullptr };
            mutable std::shared_mutex mutex;

            void reallocate(size_t new_capacity)
            {
                auto current_data = data.load();
                auto new_data = std::make_unique<T[]>(new_capacity);

                std::copy_n(
                    std::make_move_iterator(current_data.get()),
                    std::min(current_size.load(), new_capacity),
                    new_data.get()
                );

                data.store(std::move(new_data));
                capacity = new_capacity;
            }

        public:
            class iterator
            {
            private:
                T* ptr;
            public:
                using iterator_category = std::random_access_iterator_tag;

                iterator(T* p) : ptr(p)
                {  }

                T& operator*()
                {
                    return *ptr;
                }
                T* operator->()
                {
                    return ptr;
                }

                iterator& operator++()
                {
                    ++ptr;
                    return *this;
                }
                iterator operator++(int)
                {
                    iterator tmp = *this;
                    ++ptr;
                    return tmp;
                }

                iterator& operator--()
                {
                    --ptr;
                    return *this;
                }
                iterator operator--(int)
                {
                    iterator tmp = *this;
                    --ptr;
                    return tmp;
                }

                iterator& operator+=(std::ptrdiff_t n)
                {
                    ptr += n;
                    return *this;
                }
                iterator& operator-=(std::ptrdiff_t n)
                {
                    ptr -= n;
                    return *this;
                }

                iterator operator+(std::ptrdiff_t n) const
                {
                    return iterator(ptr + n);
                }
                iterator operator-(std::ptrdiff_t n) const
                {
                    return iterator(ptr - n);
                }

                std::ptrdiff_t operator-(const iterator& other) const
                {
                    return ptr - other.ptr;
                }

                bool operator==(const iterator& other) const
                {
                    return ptr == other.ptr;
                }
                bool operator!=(const iterator& other) const
                {
                    return ptr != other.ptr;
                }
                bool operator<(const iterator& other) const
                {
                    return ptr < other.ptr;
                }
            };

            class const_iterator
            {
            private:
                T* ptr;
            public:
                using iterator_category = std::random_access_iterator_tag;

                const_iterator(T* p) : ptr(p)
                {  }

                const T& operator*() const
                {
                    return *ptr;
                }
                T* operator->() const
                {
                    return ptr;
                }

                const_iterator& operator++()
                {
                    ++ptr;
                    return *this;
                }
                const_iterator operator++(int)
                {
                    const_iterator tmp = *this;
                    ++ptr;
                    return tmp;
                }

                const_iterator& operator--()
                {
                    --ptr;
                    return *this;
                }
                const_iterator operator--(int)
                {
                    const_iterator tmp = *this;
                    --ptr;
                    return tmp;
                }

                const_iterator& operator+=(std::ptrdiff_t n)
                {
                    ptr += n;
                    return *this;
                }
                const_iterator& operator-=(std::ptrdiff_t n)
                {
                    ptr -= n;
                    return *this;
                }

                const_iterator operator+(std::ptrdiff_t n) const
                {
                    return const_iterator(ptr + n);
                }
                const_iterator operator-(std::ptrdiff_t n) const
                {
                    return const_iterator(ptr - n);
                }

                std::ptrdiff_t operator-(const const_iterator& other) const
                {
                    return ptr - other.ptr;
                }

                bool operator==(const const_iterator& other) const
                {
                    return ptr == other.ptr;
                }
                bool operator!=(const const_iterator& other) const
                {
                    return ptr != other.ptr;
                }
                bool operator<(const const_iterator& other) const
                {
                    return ptr < other.ptr;
                }
            };

            ThreadSafeVector() : data(std::make_unique<T[]>(4)), capacity(4)
            {  }

            explicit ThreadSafeVector(size_t initial_capacity)
                : capacity(initial_capacity), data(std::make_unique<T[]>(initial_capacity))
            {  }

            ThreadSafeVector(ThreadSafeVector&& other) noexcept
            {
                std::unique_lock lock(mutex);
                current_size.store(other.current_size.load());
                capacity.store(other.capacity.load());
                data.store(std::move(other.data.load()));
                other.current_size.store(0);
                other.capacity.store(0);
            }

            ThreadSafeVector(const ThreadSafeVector& other)
            {
                std::shared_lock read_lock(other.mutex);
                current_size.store(other.current_size.load());
                capacity.store(other.capacity.load());

                auto new_data = std::make_unique<T[]>(capacity.load());
                auto source_data = other.data.load();

                std::copy_n(source_data.get(), current_size.load(), new_data.get());
                data.store(std::move(new_data));
            }

            void reserve(size_t new_capacity)
            {
                std::unique_lock lock(mutex);
                if (new_capacity > capacity.load()) {
                    reallocate(new_capacity);
                }
            }

            void shrink_to_fit()
            {
                std::unique_lock lock(mutex);
                if (current_size.load() < capacity.load()) {
                    reallocate(current_size.load());
                }
            }

            void push_back(const T& value)
            {
                std::unique_lock lock(mutex);
                size_t current_cap = capacity.load();

                if (current_size >= current_cap)
                {
                    reallocate(current_cap * 2);
                }

                data.load()[current_size++] = value;
            }

            void push_back(T&& value)
            {
                std::unique_lock lock(mutex);
                size_t current_cap = capacity.load();

                if (current_size >= current_cap)
                {
                    reallocate(current_cap * 2);
                }

                data.load()[current_size++] = std::move(value);
            }

            T at(size_t index) const
            {
                std::shared_lock lock(mutex);

                if (index >= current_size)
                {
                    throw std::out_of_range("Error arguments: Out Of Index");
                }

                return data.load()[index];
            }

            iterator begin()
            {
                std::shared_lock lock(mutex);
                return iterator(data.load().get());
            }

            iterator end()
            {
                std::shared_lock lock(mutex);
                return iterator(data.load().get() + current_size);
            }

            const_iterator begin() const
            {
                std::shared_lock lock(mutex);
                return const_iterator(data.load().get());
            }

            const_iterator end() const
            {
                std::shared_lock lock(mutex);
                return const_iterator(data.load().get() + current_size);
            }

            const_iterator cbegin() const
            {
                return begin();
            }
            const_iterator cend() const
            {
                return end();
            }

            size_t size() const
            {
                return current_size.load();
            }

            bool empty() const
            {
                return current_size.load() == 0;
            }

            void clear()
            {
                std::unique_lock lock(mutex);
                current_size = 0;
            }

            void pop_back()
            {
                std::unique_lock lock(mutex);

                if (current_size > 0)
                {
                    --current_size;
                }
            }
        };
    }
}
#endif // VISUALNOVEL_TSVECTOR_H