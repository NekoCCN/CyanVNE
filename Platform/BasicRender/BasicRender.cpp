#include "BasicRender.h"

bool cyanvne::platform::basicrender::ResponsiveUtils::universalMouseEventListen(const SDL_Event* event,
	MouseEvent event_type, const SDL_FRect& rect)
{
	if (event_type == MouseEvent::MOUSE_BUTTON_DOWN)
	{
		if (event->type != SDL_EVENT_MOUSE_BUTTON_DOWN)
		{
			return false;
		}
	}
	else if (event_type == MouseEvent::MOUSE_BUTTON_UP)
	{
		if (event->type != SDL_EVENT_MOUSE_BUTTON_UP)
		{
			return false;
		}
	}
	else if (event_type == MouseEvent::MOUSE_MOTION)
	{
		if (event->type != SDL_EVENT_MOUSE_MOTION)
		{
			return false;
		}
	}

	float x, y;
	SDL_GetMouseState(&x, &y);


	if (x < rect.x)
	{
		return false;
	}
	else if (x > rect.x + rect.w)
	{
		return false;
	}
	else if (y < rect.y)
	{
		return false;
	}
	else if (y > rect.y + rect.h)
	{
		return false;
	}

	return true;
}
