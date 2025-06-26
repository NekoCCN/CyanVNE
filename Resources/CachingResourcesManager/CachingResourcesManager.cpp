#include "CachingResourcesManager.h"
#include <Resources/ResourcesException/ResourcesException.h>

cyanvne::resources::AssetHandle::AssetHandle(CachingResourcesManager* manager, const uint64_t id, const std::vector<uint8_t>* data)
    : manager_(manager), resource_id_(id), resource_data_ptr_(data) {}

cyanvne::resources::AssetHandle::~AssetHandle()
{
    if (manager_) manager_->releaseResource(resource_id_);
}

cyanvne::resources::AssetHandle::AssetHandle(AssetHandle&& other) noexcept
    : manager_(other.manager_), resource_id_(other.resource_id_), resource_data_ptr_(other.resource_data_ptr_)
{
    other.manager_ = nullptr;
}

cyanvne::resources::AssetHandle& cyanvne::resources::AssetHandle::operator=(cyanvne::resources::AssetHandle&& other) noexcept
{
    if (this != &other)
    {
        if (manager_) manager_->releaseResource(resource_id_);
        manager_ = other.manager_;
        resource_id_ = other.resource_id_;
        resource_data_ptr_ = other.resource_data_ptr_;
        other.manager_ = nullptr;
    }
    return *this;
}

const std::vector<uint8_t>& cyanvne::resources::AssetHandle::getData() const
{
    if (!resource_data_ptr_) throw std::runtime_error("AssetHandle is empty or moved.");
    return *resource_data_ptr_;
}

const std::vector<uint8_t>* cyanvne::resources::AssetHandle::operator->() const
{
    return resource_data_ptr_;
}

cyanvne::resources::AssetHandle::operator bool() const
{
    return manager_ != nullptr && resource_data_ptr_ != nullptr;
}

void cyanvne::resources::CachingResourcesManager::releaseResource(const uint64_t id)
{
    std::lock_guard<std::mutex> lock(cache_mutex_);
    auto it = cache_map_.find(id);
    if (it != cache_map_.end() && it->second.ref_count > 0)
    {
        it->second.ref_count--;
    }
}

void cyanvne::resources::CachingResourcesManager::evictFromA1()
{
	while (current_a1_size_ > target_a1_size_ && !a1_in_queue_.empty())
	{
		bool evicted = false;
		for (auto it = std::prev(a1_in_queue_.end()); ; --it)
		{
			uint64_t id_to_evict = *it;
			auto map_it = cache_map_.find(id_to_evict);

			if (map_it != cache_map_.end() && map_it->second.ref_count == 0)
			{
				current_a1_size_ -= map_it->second.data.size();
				current_volatile_size_ -= map_it->second.data.size();
				a1_in_queue_.erase(it);
				cache_map_.erase(map_it);
				evicted = true;
				break;
			}

			if (it == a1_in_queue_.begin())
				break;
		}

		if (!evicted) break;
	}
}

void cyanvne::resources::CachingResourcesManager::evictFromAMain()
{
	size_t current_am_size = current_volatile_size_ - current_a1_size_;
	size_t max_am_size = max_volatile_size_ - target_a1_size_;

	while (current_am_size > max_am_size && !a_main_queue_.empty())
	{
		bool evicted = false;
		for (auto it = std::prev(a_main_queue_.end()); ; --it)
		{
			uint64_t id_to_evict = *it;
			auto map_it = cache_map_.find(id_to_evict);

			if (map_it != cache_map_.end() && map_it->second.ref_count == 0)
			{
				current_volatile_size_ -= map_it->second.data.size();
				a_main_queue_.erase(it);
				cache_map_.erase(map_it);
				current_am_size -= map_it->second.data.size();
				evicted = true;
				break;
			}

			if (it == a_main_queue_.begin())
				break;
		}

		if (!evicted)
			break;
	}
}

cyanvne::resources::AssetHandle cyanvne::resources::CachingResourcesManager::getResource(const std::string& alias, const bool as_persistent)
{
    const ResourceDefinition* def = base_manager_->getDefinitionByAlias(alias);
    if (!def) throw std::runtime_error("Resource not found with alias: " + alias);
    return getResource(def->id, as_persistent);
}

cyanvne::resources::AssetHandle cyanvne::resources::CachingResourcesManager::getResource(uint64_t id, const bool as_persistent)
{
    std::lock_guard<std::mutex> lock(cache_mutex_);

    auto it = cache_map_.find(id);

    if (it != cache_map_.end())
    {
        CacheEntry& entry = it->second;
        entry.ref_count++;

        if (entry.location == CacheLocation::IN_A1)
        {
            a1_in_queue_.erase(entry.queue_iterator);
            current_a1_size_ -= entry.data.size();

            a_main_queue_.push_front(id);
            entry.location = CacheLocation::IN_AMAIN;
            entry.queue_iterator = a_main_queue_.begin();

            evictFromAMain();
        }
        else if (entry.location == CacheLocation::IN_AMAIN)
        {
            a_main_queue_.splice(a_main_queue_.begin(), a_main_queue_, entry.queue_iterator);
        }

        return { this, id, &entry.data };
    }

    auto data = base_manager_->getResourceDataById(id);
    size_t data_size = data.size();

    if (as_persistent && data_size <= max_single_persistent_size_ && (current_persistent_size_ + data_size) <= max_persistent_size_)
    {
        CacheEntry new_entry;
        new_entry.data = std::move(data);
        new_entry.ref_count = 1;
        new_entry.location = CacheLocation::PERSISTENT;

        auto [inserted_it, success] = cache_map_.emplace(id, std::move(new_entry));
        current_persistent_size_ += data_size;
        return { this, id, &inserted_it->second.data };
    }

    if (current_volatile_size_ + data_size > max_volatile_size_)
    {
        evictFromA1();
        evictFromAMain();
    }

    if (current_volatile_size_ + data_size > max_volatile_size_)
    {
        throw exception::MemoryAllocException("Not enough volatile cache space for resource ID: " + std::to_string(id));
    }

    evictFromA1();

    a1_in_queue_.push_front(id);
    CacheEntry new_entry;
    new_entry.data = std::move(data);
    new_entry.ref_count = 1;
    new_entry.location = CacheLocation::IN_A1;
    new_entry.queue_iterator = a1_in_queue_.begin();

    auto [inserted_it, success] = cache_map_.emplace(id, std::move(new_entry));
    current_a1_size_ += data_size;
    current_volatile_size_ += data_size;

    return { this, id, &inserted_it->second.data };
}