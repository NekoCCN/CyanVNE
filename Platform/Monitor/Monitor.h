#pragma once
#include <SDL3/SDL.h>
#include <cstdint>
#include <vector>
#include <string>
#include <Platform/PlatformException/PlatformException.h>

namespace cyanvne
{
	namespace platform
	{
		class MonitorLists
		{
		private:
			int32_t monitor_num_;
			SDL_DisplayID* monitor_list_;
		public:
			MonitorLists()
			{
				monitor_num_ = SDL_GetNumVideoDrivers();
				if (monitor_num_ < 1)
				{
					throw exception::platformexception::MonitorNotFoundException();
				}
				monitor_list_ = SDL_GetDisplays(nullptr);
			}
			int32_t getMonitorNum() const
			{
				return monitor_num_;
			}
			SDL_Rect getDisplayBound(int32_t index) const;  // index from 0 to monitor num - 1
			void getDisplayBound(int32_t index, SDL_Rect* pt) const;
			void refresh()
			{
				monitor_num_ = SDL_GetNumVideoDrivers();
				if (monitor_num_ < 1)
				{
					throw exception::platformexception::MonitorNotFoundException();
				}
				monitor_list_ = SDL_GetDisplays(nullptr);
			}
			std::vector<SDL_DisplayID> getMonitorLists() const
			{
				return std::vector<SDL_DisplayID>(monitor_list_, monitor_list_ + monitor_num_ - 1);
			}
			const char* getMonitorName(int32_t index) const
			{
				if (monitor_num_ < 1)
				{
					throw exception::platformexception::MonitorNotFoundException();
				}
				return SDL_GetDisplayName(monitor_list_[index]);
			}
			std::vector<std::string> getMonitorNameList() const;
			SDL_DisplayID operator[](int index) const;
			std::vector<SDL_Rect> getMonitorBounds() const
			{
				std::vector<SDL_Rect> tmp;
				for (int i = 0; i < monitor_num_; i++)
				{
					tmp.emplace_back();
					SDL_GetDisplayBounds(i, &(tmp.back()));
				}
				return tmp;
			}
		};
	}
}