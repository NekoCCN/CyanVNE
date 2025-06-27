#pragma once
#include <list>
#include <unordered_map>
#include <mutex>
#include <memory>
#include <string>
#include <cstdint>
#include <stdexcept>
#include <typeinfo>
#include <Resources/ResourcesManager/ResourcesManager.h>
#include <Resources/ResourcesException/ResourcesException.h>
#include <Resources/ResourceTypes/ResourceTypes.h>

namespace cyanvne
{
    namespace resources
    {
        class UnifiedCacheManager;

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
            enum class CacheLocation : uint8_t
            {
                IN_A1,
                IN_AMAIN
            };

            struct CacheEntry
            {
                std::unique_ptr<ICachedResource> resource;
                std::list<uint64_t>::iterator queue_iterator;
                size_t ref_count = 0;
                CacheLocation location;
            };

            using CacheIterator = std::unordered_map<uint64_t, CacheEntry>::iterator;

            void releaseResource(uint64_t key);
            template<typename T> std::unique_ptr<T> loadResource(uint64_t id);
            void promote(const CacheIterator& it);
            void evictEntry(const CacheIterator& it);
            bool evictOne();

            std::shared_ptr<ResourcesManager> base_manager_;
            SDL_Renderer* renderer_;
            std::unordered_map<uint64_t, CacheEntry> cache_map_;
            std::list<uint64_t> a1_in_queue_;
            std::list<uint64_t> a_main_queue_;
            size_t max_size_bytes_;
            size_t current_size_bytes_ = 0;
            size_t target_a1_size_bytes_;
            mutable std::mutex cache_mutex_;

        public:
            template<typename T> friend class ResourceHandle;

            explicit UnifiedCacheManager(const std::shared_ptr<ResourcesManager>& base_manager, SDL_Renderer* renderer, size_t max_size_bytes, float a1_ratio = 0.25f);
            ~UnifiedCacheManager() = default;

            UnifiedCacheManager(const UnifiedCacheManager&) = delete;
            UnifiedCacheManager& operator=(const UnifiedCacheManager&) = delete;
            UnifiedCacheManager(UnifiedCacheManager&&) = delete;
            UnifiedCacheManager& operator=(UnifiedCacheManager&&) = delete;

            template <typename T> ResourceHandle<T> get(uint64_t id);
            template <typename T> ResourceHandle<T> get(const std::string& alias);

            uint64_t getMaxCacheBufferSize() const
            {
                return max_size_bytes_;
            }
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

        template <typename T>
        ResourceHandle<T> UnifiedCacheManager::get(uint64_t id)
        {
            std::lock_guard<std::mutex> lock(cache_mutex_);

            auto it = cache_map_.find(id);

            if (it != cache_map_.end())
            {
	            if (T* resource = dynamic_cast<T*>(it->second.resource.get()))
                {
                    promote(it);
                    return ResourceHandle<T>(this, id, resource);
                }

                throw exception::resourcesexception::ResourceManagerIOException(
                    "Type mismatch for cached resource ID: " + std::to_string(id) +
                    ". Requested " + typeid(T).name() +
                    ", but cache holds " + typeid(it->second.resource).name());
            }

            std::unique_ptr<T> new_resource = loadResource<T>(id);
            if (!new_resource)
            {
                throw std::runtime_error("Failed to load resource with ID: " + std::to_string(id));
            }

            size_t resource_size = new_resource->getSizeInBytes();

            if (resource_size > max_size_bytes_)
            {
                throw exception::MemoryAllocException("Resource is larger than the total cache size. ID: " + std::to_string(id));
            }
            while (current_size_bytes_ + resource_size > max_size_bytes_)
            {
                if (!evictOne())
                {
                    throw exception::MemoryAllocException("Not enough cache space for resource and nothing can be evicted. ID: " + std::to_string(id));
                }
            }

            a1_in_queue_.push_front(id);
            CacheEntry new_entry;
            new_entry.resource = std::move(new_resource);
            new_entry.ref_count = 1;
            new_entry.location = CacheLocation::IN_A1;
            new_entry.queue_iterator = a1_in_queue_.begin();

            auto [inserted_it, success] = cache_map_.emplace(id, std::move(new_entry));
            current_size_bytes_ += resource_size;

            T* resource_ptr = dynamic_cast<T*>(inserted_it->second.resource.get());

            return ResourceHandle<T>(this, id, resource_ptr);
        }

        template <typename T>
        ResourceHandle<T> UnifiedCacheManager::get(const std::string& alias)
        {
            const ResourceDefinition* def = base_manager_->getDefinitionByAlias(alias);
            if (!def)
            {
                throw std::runtime_error("Resource not found with alias: " + alias);
            }
            return get<T>(def->id);
        }

        template<>
        inline std::unique_ptr<RawDataResource> UnifiedCacheManager::loadResource<RawDataResource>(uint64_t id)
        {
            return std::make_unique<RawDataResource>(id, base_manager_.get());
        }

        template<>
        inline std::unique_ptr<TextureResource> UnifiedCacheManager::loadResource<TextureResource>(uint64_t id)
        {
            auto raw_data_resource = this->loadResource<RawDataResource>(id);
            return std::make_unique<TextureResource>(raw_data_resource->data, renderer_);
        }

        template<>
        inline std::unique_ptr<SoLoudWavResource> UnifiedCacheManager::loadResource<SoLoudWavResource>(uint64_t id)
        {
            auto raw_data_resource = this->loadResource<RawDataResource>(id);
            return std::make_unique<SoLoudWavResource>(raw_data_resource->data);
        }
    }
}
