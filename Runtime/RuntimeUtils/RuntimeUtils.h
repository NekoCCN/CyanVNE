#pragma once
#include <SDL3/SDL.h>

namespace cyanvne
{
	namespace runtime
	{
		namespace utils
		{
			SDL_Rect textureToRect(const SDL_Texture* texture);

			SDL_Rect geometricScalingRectByX(const SDL_Rect& rect, double dst_w);

			SDL_FRect geometricScalingFRectByX(const SDL_FRect& rect, double dst_w);

			SDL_Rect geometricScalingRectByY(const SDL_Rect& rect, double dst_h);

			SDL_FRect geometricScalingFRectByY(const SDL_FRect& rect, double dst_h);

			SDL_Rect geometricScalingRectByX(const SDL_Rect& window_rect, const SDL_Rect& rect, double w_ratio);

			SDL_FRect geometricScalingFRectByX(const SDL_Rect& window_rect, SDL_FRect& rect, double w_ratio);

			SDL_Rect geometricScalingRectByY(const SDL_Rect& window_rect, const SDL_Rect& rect, double h_ratio);

			SDL_FRect geometricScalingFRectByY(const SDL_Rect& window_rect, const SDL_FRect& rect, double h_ratio);
		}
	}
}