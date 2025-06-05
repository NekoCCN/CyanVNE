#include "RuntimeUtils.h"

SDL_Rect cyanvne::runtime::utils::textureToRect(const SDL_Texture* texture)
{
	return {
		0,
		0,
		texture->w,
		texture->h
	};
}

SDL_Rect cyanvne::runtime::utils::geometricScalingRectByX(const SDL_Rect& rect, double dst_w)
{
	double ratio = dst_w / rect.w;

	return {
		static_cast<int>(rect.x * ratio),
		static_cast<int>(rect.y * ratio),
		static_cast<int>(rect.w * ratio),
		static_cast<int>(rect.h * ratio)
	};
}

SDL_FRect cyanvne::runtime::utils::geometricScalingFRectByX(const SDL_FRect& rect, double dst_w)
{
	double ratio = dst_w / rect.w;

	return {
		static_cast<float>(rect.x * ratio),
		static_cast<float>(rect.y * ratio),
		static_cast<float>(rect.w * ratio),
		static_cast<float>(rect.h * ratio)
	};
}

SDL_Rect cyanvne::runtime::utils::geometricScalingRectByY(const SDL_Rect& rect, double dst_h)
{
	double ratio = dst_h / rect.h;

	return {
		static_cast<int>(rect.x * ratio),
		static_cast<int>(rect.y * ratio),
		static_cast<int>(rect.w * ratio),
		static_cast<int>(rect.h * ratio)
	};
}

SDL_FRect cyanvne::runtime::utils::geometricScalingFRectByY(const SDL_FRect& rect, double dst_h)
{
	double ratio = dst_h / rect.h;

	return {
		static_cast<float>(rect.x * ratio),
		static_cast<float>(rect.y * ratio),
		static_cast<float>(rect.w * ratio),
		static_cast<float>(rect.h * ratio)
	};
}

SDL_Rect cyanvne::runtime::utils::geometricScalingRectByX(const SDL_Rect& window_rect, const SDL_Rect& rect, double w_ratio)
{
	double ratio = window_rect.w * w_ratio / rect.w;

	return {
		static_cast<int>(rect.x * ratio),
		static_cast<int>(rect.y * ratio),
		static_cast<int>(rect.w * ratio),
		static_cast<int>(rect.h * ratio)
	};
}

SDL_FRect cyanvne::runtime::utils::geometricScalingFRectByX(const SDL_Rect& window_rect, SDL_FRect& rect, double w_ratio)
{
	double ratio = window_rect.w * w_ratio / rect.w;

	return {
		static_cast<float>(rect.x * ratio),
		static_cast<float>(rect.y * ratio),
		static_cast<float>(rect.w * ratio),
		static_cast<float>(rect.h * ratio)
	};
}

SDL_Rect cyanvne::runtime::utils::geometricScalingRectByY(const SDL_Rect& window_rect, const SDL_Rect& rect, double h_ratio)
{
	double ratio = window_rect.h * h_ratio / rect.h;

	return {
		static_cast<int>(rect.x * ratio),
		static_cast<int>(rect.y * ratio),
		static_cast<int>(rect.w * ratio),
		static_cast<int>(rect.h * ratio)
	};
}

SDL_FRect cyanvne::runtime::utils::geometricScalingFRectByY(const SDL_Rect& window_rect, const SDL_FRect& rect, double h_ratio)
{
	double ratio = window_rect.h * h_ratio / rect.h;

	return {
		static_cast<float>(rect.x * ratio),
		static_cast<float>(rect.y * ratio),
		static_cast<float>(rect.w * ratio),
		static_cast<float>(rect.h * ratio)
	};
}