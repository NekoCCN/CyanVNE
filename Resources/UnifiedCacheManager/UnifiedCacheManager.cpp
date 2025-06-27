#include "UnifiedCacheManager.h"
#include <SDL3_image/SDL_image.h>
#include <stdexcept>


namespace cyanvne
{
    namespace resources
    {
        UnifiedCacheManager::UnifiedCacheManager(const std::shared_ptr<ResourcesManager>& base_manager, SDL_Renderer* renderer, size_t max_size_bytes, float a1_ratio)
            : base_manager_(base_manager),
            renderer_(renderer),
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
            std::lock_guard<std::mutex> lock(cache_mutex_);
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
    }
}
