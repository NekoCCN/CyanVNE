#pragma once
#include <list>
#include <unordered_map>
#include <mutex>
#include <stdexcept>
#include <memory>
#include <Resources/ResourcesManager/ResourcesManager.h>

namespace cyanvne
{
    namespace resources
    {
        class CachingResourcesManager;

        class AssetHandle
        {
        private:
            CachingResourcesManager* manager_;
            uint64_t resource_id_;
            const std::vector<uint8_t>* resource_data_ptr_;

        public:
            AssetHandle(CachingResourcesManager* manager, uint64_t id, const std::vector<uint8_t>* data);
            ~AssetHandle();

            AssetHandle(const AssetHandle&) = delete;
            AssetHandle& operator=(const AssetHandle&) = delete;
            AssetHandle(AssetHandle&& other) noexcept;
            AssetHandle& operator=(AssetHandle&& other) noexcept;

            const std::vector<uint8_t>& getData() const;
            const std::vector<uint8_t>* operator->() const;
            explicit operator bool() const;
        };

        class CachingResourcesManager
        {
        private:
            std::shared_ptr<ResourcesManager> base_manager_;

            enum class CacheLocation : uint8_t
            {
                IN_A1, IN_AMAIN, PERSISTENT
            };

            struct CacheEntry
            {
                std::vector<uint8_t> data;
                std::list<uint64_t>::iterator queue_iterator;
                size_t ref_count = 0;
                CacheLocation location;
            };

            std::unordered_map<uint64_t, CacheEntry> cache_map_;

            std::list<uint64_t> a1_in_queue_;
            std::list<uint64_t> a_main_queue_;

            size_t max_volatile_size_;
            size_t current_volatile_size_ = 0;
            size_t target_a1_size_;
            size_t current_a1_size_ = 0;

            size_t max_persistent_size_;
            size_t current_persistent_size_ = 0;
            size_t max_single_persistent_size_;

            mutable std::mutex cache_mutex_;
            friend class AssetHandle;

            void releaseResource(uint64_t id);

            void evictFromA1();
            void evictFromAMain();

        public:
            explicit CachingResourcesManager(
                const std::shared_ptr<ResourcesManager>& base_manager,
                const size_t max_volatile_size,
                const size_t max_persistent_size,
                const size_t max_single_persistent_size,
                const float a1_ratio = 0.25f)
                : base_manager_(base_manager),
                max_volatile_size_(max_volatile_size),
                target_a1_size_(static_cast<size_t>(static_cast<double>(max_volatile_size)* a1_ratio)),
                max_persistent_size_(max_persistent_size),
                max_single_persistent_size_(max_single_persistent_size)
            {
                if (!base_manager_ || !base_manager_->isInitialized())
                {
                    throw std::invalid_argument("Provided base ResourcesManager is not valid or not initialized.");
                }
            }

            CachingResourcesManager(const CachingResourcesManager&) = delete;
            CachingResourcesManager(CachingResourcesManager&&) = delete;
            CachingResourcesManager& operator=(const CachingResourcesManager&) = delete;
            CachingResourcesManager& operator=(CachingResourcesManager&&) = delete;

            AssetHandle getResource(const std::string& alias, bool as_persistent = false);
            AssetHandle getResource(uint64_t id, bool as_persistent = false);

            ~CachingResourcesManager() = default;
        };
    };
}
