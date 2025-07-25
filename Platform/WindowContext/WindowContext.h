#pragma once

#include <SDL3/SDL.h>
#include <cstdint>
#include <chrono>
#include <bgfx/bgfx.h>

namespace cyanvne
{
	namespace platform
	{
		class WindowContext
		{
		public:
			WindowContext(
				const char* title,
				uint32_t width = 1280,
				uint32_t height = 720,
				uint32_t target_fps = 0
			);
			~WindowContext();

			WindowContext(const WindowContext&) = delete;
			WindowContext& operator=(const WindowContext&) = delete;
			WindowContext(WindowContext&&) = delete;
			WindowContext& operator=(WindowContext&&) = delete;

			void reset(uint32_t width, uint32_t height);

			SDL_Window* getWindowHandle() const;
			bgfx::FrameBufferHandle getFrameBufferHandle() const;
			std::chrono::duration<double> getTargetFrameDuration() const;
			void getWindowSize(int32_t* width, int32_t* height) const;

			void showWindow();
			void hideWindow();
			void setWindowPosition(int32_t x, int32_t y);
			void setWindowSize(int32_t width, int32_t height);
			void setWindowFullscreen(bool fullscreen);
			bool isMinimized() const;
			bool isOccluded() const;
		private:
			void* getNativeWindowHandle();

			SDL_Window* window_;
			uint32_t width_;
			uint32_t height_;
			std::chrono::duration<double> target_frame_duration_;
			bgfx::FrameBufferHandle frame_buffer_handle_;
		};
	}
}