#pragma execution_character_set("utf-8")
#include "Monitor.h"
#include <ranges>

// Notice Exception Handling
SDL_Rect cyanvne::platform::MonitorLists::getDisplayBound(int32_t index) const
{
	SDL_Rect tmp;
	if (!(index <= monitor_num_ - 1 && index >= 0))  //index from 0 to monitor num - 1
	{
		throw exception::IllegalArgumentException("Invalid monitor index: %d < 0 or %d > monitor_num(%d) - 1");
	}
	SDL_GetDisplayBounds(monitor_list_[index], &tmp);
	return tmp;
}
// Notice Exception Handling
void cyanvne::platform::MonitorLists::getDisplayBound(int32_t index, SDL_Rect* pt) const
{
	if (!(index <= monitor_num_ - 1 && index >= 0))  //index from 0 to monitor num - 1
	{
		throw exception::IllegalArgumentException("Invalid monitor index: %d < 0 or %d > monitor_num(%d) - 1");
	}
	SDL_GetDisplayBounds(monitor_list_[index], pt);
}
std::vector<std::string> cyanvne::platform::MonitorLists::getMonitorNameList() const
{
	// 0 - (monitor_num_ - 1)
	return std::views::iota(0, monitor_num_) | std::views::transform([this](int i)
		{
			return std::string(SDL_GetDisplayName(monitor_list_[i]));
		}) | std::ranges::to<std::vector>();

}
// Notice Exception Handling
SDL_DisplayID cyanvne::platform::MonitorLists::operator[](int index) const
{
	if (!(index <= monitor_num_ - 1 && index >= 0))  // index from 0 to monitor num - 1
	{
		throw exception::IllegalArgumentException("Invalid monitor index: %d < 0 or %d > monitor_num(%d) - 1");
	}
	return monitor_list_[index];
}
