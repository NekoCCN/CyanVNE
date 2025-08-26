#include "UnifiedCacheManager.h"
#include <stdexcept>
#include <utility>

namespace cyanvne
{
    namespace resources
    {
        UnifiedCacheManager::UnifiedCacheManager(const std::shared_ptr<ResourcesManager>& base_manager, size_t max_size_bytes, float a1_ratio)
                : base_manager_(base_manager),
                  max_size_bytes_(max_size_bytes),
                  current_size_bytes_(0),
                  target_a1_size_bytes_(static_cast<size_t>(static_cast<double>(max_size_bytes)* a1_ratio))
        {
            if (!base_manager_ || !base_manager_->isInitialized())
            {
                throw std::invalid_argument("Base manager is not valid or not initialized.");
            }
        }

        void UnifiedCacheManager::releaseResource(uint64_t key)
        {
            std::lock_guard<std::recursive_mutex> lock(cache_mutex_);
            auto it = cache_map_.find(key);
            if (it != cache_map_.end() && it->second.ref_count > 0)
            {
                it->second.ref_count--;
            }
        }

        void UnifiedCacheManager::promote(const CacheIterator& it)
        {
            it->second.ref_count++;
            if (it->second.location == CacheLocation::IN_A1)
            {
                a1_in_queue_.erase(it->second.queue_iterator);
                a_main_queue_.push_front(it->first);
                it->second.location = CacheLocation::IN_AMAIN;
                it->second.queue_iterator = a_main_queue_.begin();
            }
            else
            {
                a_main_queue_.splice(a_main_queue_.begin(), a_main_queue_, it->second.queue_iterator);
            }
        }

        void UnifiedCacheManager::evictEntry(const CacheIterator& it)
        {
            current_size_bytes_ -= it->second.resource->getSizeInBytes();
            if (it->second.location == CacheLocation::IN_A1)
            {
                a1_in_queue_.erase(it->second.queue_iterator);
            }
            else
            {
                a_main_queue_.erase(it->second.queue_iterator);
            }
            cache_map_.erase(it);
        }

        bool UnifiedCacheManager::evictOne()
        {
            auto evict_from = [&](std::list<uint64_t>& queue)
            {
                if (queue.empty())
                {
                    return false;
                }
                for (auto it = std::prev(queue.end()); ; --it)
                {
                    auto map_it = cache_map_.find(*it);
                    if (map_it != cache_map_.end() && map_it->second.ref_count == 0)
                    {
                        evictEntry(map_it);
                        return true;
                    }
                    if (it == queue.begin())
                    {
                        break;
                    }
                }
                return false;
            };

            if (evict_from(a_main_queue_))
            {
                return true;
            }
            if (evict_from(a1_in_queue_))
            {
                return true;
            }
            return false;
        }

        template <typename T>
        ResourceHandle<T> UnifiedCacheManager::get(uint64_t id)
        {
            std::lock_guard<std::recursive_mutex> lock(cache_mutex_);

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

        ResourceHandle<TextureResource> UnifiedCacheManager::get(uint64_t id, ImageLoader loader)
        {
            std::lock_guard<std::recursive_mutex> lock(cache_mutex_);

            auto it = cache_map_.find(id);

            if (it != cache_map_.end())
            {
                if (TextureResource* resource = dynamic_cast<TextureResource*>(it->second.resource.get()))
                {
                    promote(it);
                    return ResourceHandle<TextureResource>(this, id, resource);
                }

                throw exception::resourcesexception::ResourceManagerIOException(
                        "Type mismatch for cached resource ID: " + std::to_string(id) +
                        ". Requested TextureResource, but cache holds different type.");
            }

            std::unique_ptr<TextureResource> new_resource = loadResource(id, loader);
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

            auto* resource_ptr = dynamic_cast<TextureResource*>(inserted_it->second.resource.get());

            return ResourceHandle<TextureResource>(this, id, resource_ptr);
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

        ResourceHandle<TextureResource> UnifiedCacheManager::get(const std::string& alias, ImageLoader loader)
        {
            const ResourceDefinition* def = base_manager_->getDefinitionByAlias(alias);
            if (!def)
            {
                throw std::runtime_error("Resource not found with alias: " + alias);
            }
            return get(def->id, loader);
        }

        PinnedResourceHandle UnifiedCacheManager::getUncachedBuffer(uint64_t id)
        {
            std::vector<uint8_t> data = base_manager_->getResourceDataById(id);
            if (data.empty()) {
                throw exception::resourcesexception::ResourceManagerIOException(
                        "Failed to load resource data for PinnedResourceHandle, ID: " + std::to_string(id)
                );
            }
            return PinnedResourceHandle(std::move(data));
        }

        PinnedResourceHandle UnifiedCacheManager::getUncachedBuffer(const std::string& alias)
        {
            const ResourceDefinition* def = base_manager_->getDefinitionByAlias(alias);
            if (!def)
            {
                throw std::runtime_error("Resource not found with alias: " + alias);
            }
            return getUncachedBuffer(def->id);
        }

        template<>
        inline std::unique_ptr<RawDataResource> UnifiedCacheManager::loadResource<RawDataResource>(uint64_t id)
        {
            return std::make_unique<RawDataResource>(id, base_manager_.get());
        }

        inline std::unique_ptr<TextureResource> UnifiedCacheManager::loadResource(uint64_t id, ImageLoader loader)
        {
            auto raw_data_handle = get<RawDataResource>(id);
            return std::make_unique<TextureResource>(raw_data_handle->data, loader);
        }

        template<>
        inline std::unique_ptr<TextureResource> UnifiedCacheManager::loadResource<TextureResource>(uint64_t id)
        {
            return loadResource(id, ImageLoader::INTERNAL);
        }

        template<>
        inline std::unique_ptr<SoLoudWavResource> UnifiedCacheManager::loadResource<SoLoudWavResource>(uint64_t id)
        {
            auto raw_data_handle = get<RawDataResource>(id);
            return std::make_unique<SoLoudWavResource>(raw_data_handle->data);
        }

        template ResourceHandle<RawDataResource> UnifiedCacheManager::get<RawDataResource>(uint64_t);
        template ResourceHandle<TextureResource> UnifiedCacheManager::get<TextureResource>(uint64_t);
        template ResourceHandle<SoLoudWavResource> UnifiedCacheManager::get<SoLoudWavResource>(uint64_t);

        template ResourceHandle<RawDataResource> UnifiedCacheManager::get<RawDataResource>(const std::string&);
        template ResourceHandle<TextureResource> UnifiedCacheManager::get<TextureResource>(const std::string&);
        template ResourceHandle<SoLoudWavResource> UnifiedCacheManager::get<SoLoudWavResource>(const std::string&);
    }
}