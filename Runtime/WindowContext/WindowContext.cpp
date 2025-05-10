#include "WindowContext.h"

using namespace cyanvne;

runtime::WindowContext::WindowContext(const char* title, uint32_t width, uint32_t height, const char* drive, SDL_WindowFlags flags)
{
	// window part
	core::GlobalLogger::getCoreLogger()->info("Try to create a window context");
	window_ = SDL_CreateWindow(title, static_cast<int>(width), static_cast<int>(height),
		flags | SDL_WINDOW_HIDDEN);
	if (window_ == nullptr)
	{
		core::GlobalLogger::getCoreLogger()->critical("Failed to create window context : {:s}", SDL_GetError());
		throw exception::runtimeexception::CreateWindowContextException("Failed to create window context");
	}

	// renderer part
	drive_num_ = SDL_GetNumRenderDrivers();
	core::GlobalLogger::getCoreLogger()->info("Supported Graphic API List: ");
	for (int i = 0; i < drive_num_; ++i)
	{
		render_drive_str_.emplace_back(SDL_GetRenderDriver(i));
		core::GlobalLogger::getCoreLogger()->info(render_drive_str_[i].c_str());
	}
	core::GlobalLogger::getCoreLogger()->info("END");
	sdl_renderer_ = SDL_CreateRenderer(window_, drive);
	if (sdl_renderer_ == nullptr)
	{
		core::GlobalLogger::getCoreLogger()->critical("Failed to create graphic context : {:s}", SDL_GetError());
		throw exception::runtimeexception::CreateWindowContextException("Failed to create graphic context");
	}
	SDL_ShowWindow(window_);
}