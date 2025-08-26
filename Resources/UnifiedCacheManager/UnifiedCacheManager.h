#pragma once
#include <list>
#include <unordered_map>
#include <mutex>
#include <memory>
#include <string>
#include <cstdint>
#include <stdexcept>
#include <typeinfo>
#include <utility>
#include <Resources/ResourcesManager/ResourcesManager.h>
#include <Resources/ResourcesException/ResourcesException.h>
#include <Resources/ResourceTypes/ResourceTypes.h>
#include "Platform/Thread/UnifiedConcurrencyManager.h"

namespace cyanvne
{
    namespace resources
    {
        class UnifiedCacheManager;
        class PinnedResourceHandle;
        template <typename T> class ResourceHandle;

        class PinnedResourceHandle
        {
        public:
            PinnedResourceHandle(const PinnedResourceHandle&) = delete;
            PinnedResourceHandle& operator=(const PinnedResourceHandle&) = delete;
            PinnedResourceHandle(PinnedResourceHandle&& other) noexcept = default;
            PinnedResourceHandle& operator=(PinnedResourceHandle&& other) noexcept = default;
            ~PinnedResourceHandle() = default;

            [[nodiscard]] const std::vector<uint8_t>& getData() const { return data_; }

        private:
            friend class UnifiedCacheManager;
            explicit PinnedResourceHandle(std::vector<uint8_t>&& data)
                    : data_(std::move(data)) {  }
            std::vector<uint8_t> data_;
        };

        template <typename T>
        class ResourceHandle
        {
        public:
            ResourceHandle(UnifiedCacheManager* manager, uint64_t key, T* resource);
            ~ResourceHandle();
            ResourceHandle(const ResourceHandle&) = delete;
            ResourceHandle& operator=(const ResourceHandle&) = delete;
            ResourceHandle(ResourceHandle&& other) noexcept;
            ResourceHandle& operator=(ResourceHandle&& other) noexcept;

            T* get() const;
            T* operator->() const;
            explicit operator bool() const;

        private:
            UnifiedCacheManager* manager_ = nullptr;
            uint64_t resource_key_;
            T* resource_ptr_ = nullptr;
            void release();
        };

        class UnifiedCacheManager
        {
        private:
            enum class CacheLocation : uint8_t { IN_A1, IN_AMAIN };
            struct CacheEntry
            {
                std::unique_ptr<ICachedResource> resource;
                std::list<uint64_t>::iterator queue_iterator;
                size_t ref_count = 0;
                CacheLocation location;
            };
            using CacheIterator = std::unordered_map<uint64_t, CacheEntry>::iterator;

            void releaseResource(uint64_t key);

            template<typename T>
            std::unique_ptr<T> loadResource(uint64_t id);
            std::unique_ptr<TextureResource> loadResource(uint64_t id, ImageLoader loader);

            void promote(const CacheIterator& it);
            void evictEntry(const CacheIterator& it);
            bool evictOne();

            std::shared_ptr<ResourcesManager> base_manager_;
            std::unordered_map<uint64_t, CacheEntry> cache_map_;
            std::list<uint64_t> a1_in_queue_;
            std::list<uint64_t> a_main_queue_;
            size_t max_size_bytes_;
            size_t current_size_bytes_ = 0;
            size_t target_a1_size_bytes_;
            mutable std::recursive_mutex cache_mutex_;

        public:
            template<typename T> friend class ResourceHandle;

            explicit UnifiedCacheManager(const std::shared_ptr<ResourcesManager>& base_manager, size_t max_size_bytes, float a1_ratio = 0.25f);
            ~UnifiedCacheManager() = default;

            UnifiedCacheManager(const UnifiedCacheManager&) = delete;
            UnifiedCacheManager& operator=(const UnifiedCacheManager&) = delete;
            UnifiedCacheManager(UnifiedCacheManager&&) = delete;
            UnifiedCacheManager& operator=(UnifiedCacheManager&&) = delete;

            template <typename T>
            ResourceHandle<T> get(uint64_t id);
            template <typename T>
            ResourceHandle<T> get(const std::string& alias);

            ResourceHandle<TextureResource> get(uint64_t id, ImageLoader loader);
            ResourceHandle<TextureResource> get(const std::string& alias, ImageLoader loader);

            PinnedResourceHandle getUncachedBuffer(uint64_t id);
            PinnedResourceHandle getUncachedBuffer(const std::string& alias);

            uint64_t getMaxCacheBufferSize() const { return max_size_bytes_; }
        };

        template <typename T>
        ResourceHandle<T>::ResourceHandle(UnifiedCacheManager* manager, uint64_t key, T* resource)
                : manager_(manager), resource_key_(key), resource_ptr_(resource)
        {  }

        template <typename T>
        ResourceHandle<T>::~ResourceHandle()
        {
            release();
        }

        template <typename T>
        ResourceHandle<T>::ResourceHandle(ResourceHandle&& other) noexcept
                : manager_(other.manager_), resource_key_(other.resource_key_), resource_ptr_(other.resource_ptr_)
        {
            other.manager_ = nullptr;
            other.resource_ptr_ = nullptr;
        }

        template <typename T>
        ResourceHandle<T>& ResourceHandle<T>::operator=(ResourceHandle&& other) noexcept
        {
            if (this != &other)
            {
                release();
                manager_ = other.manager_;
                resource_key_ = other.resource_key_;
                resource_ptr_ = other.resource_ptr_;
                other.manager_ = nullptr;
                other.resource_ptr_ = nullptr;
            }
            return *this;
        }

        template <typename T>
        T* ResourceHandle<T>::get() const
        {
            if (!resource_ptr_)
            {
                throw std::runtime_error("Handle is empty or moved.");
            }
            return resource_ptr_;
        }

        template <typename T>
        T* ResourceHandle<T>::operator->() const
        {
            return get();
        }

        template <typename T>
        ResourceHandle<T>::operator bool() const
        {
            return manager_ != nullptr && resource_ptr_ != nullptr;
        }

        template <typename T>
        void ResourceHandle<T>::release()
        {
            if (manager_)
            {
                manager_->releaseResource(resource_key_);
                manager_ = nullptr;
            }
        }
    }
}