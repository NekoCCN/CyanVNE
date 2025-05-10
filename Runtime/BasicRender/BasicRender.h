#pragma once
#include <memory>
#include <SDL3/SDL.h>

namespace cyanvne
{
	namespace runtime
	{
		namespace basicrender
		{
			class RenderableInterface
			{
			protected:
				RenderableInterface() = default;
			public:
				RenderableInterface(RenderableInterface&& other) = delete;
				RenderableInterface& operator=(RenderableInterface&& other) = delete;
				RenderableInterface(const RenderableInterface& other) = delete;
				RenderableInterface& operator=(const RenderableInterface& other) = delete;

				virtual void render() = 0;

				virtual ~RenderableInterface() = default;
			};

			class RenderStatefulInterface
			{
			protected:
				RenderStatefulInterface() = default;
			public:
				RenderStatefulInterface(RenderStatefulInterface&& other) = delete;
				RenderStatefulInterface& operator=(RenderStatefulInterface&& other) = delete;
				RenderStatefulInterface(const RenderStatefulInterface& other) = delete;
				RenderStatefulInterface& operator=(const RenderStatefulInterface& other) = delete;

				virtual void startRender() = 0;
				virtual void stopRender() = 0;
				virtual void changeRenderStatus(bool status) = 0;
				virtual bool isRender() const = 0;

				virtual ~RenderStatefulInterface() = default;
			};
			class RenderStateful : public RenderStatefulInterface
			{
			private:
				bool render_status_ = false;
			public:
				RenderStateful() = default;
				RenderStateful(bool status)
				{
					render_status_ = status;
				}
				RenderStateful(RenderStateful&& other) noexcept
				{
					render_status_ = other.render_status_;
				}
				RenderStateful& operator=(RenderStateful&& other) noexcept
				{
					render_status_ = other.render_status_;
					return *this;
				}
				RenderStateful& operator=(const RenderStateful& other)
				{
					render_status_ = other.render_status_;
					return *this;
				}
				RenderStateful(const RenderStateful& other)
				{
					render_status_ = other.render_status_;
				}

				void startRender() final
				{
					render_status_ = true;
				}
				void stopRender() final
				{
					render_status_ = false;
				}
				void changeRenderStatus(bool status) final
				{
					render_status_ = status;
				}
				bool isRender() const final
				{
					return render_status_;
				}

				~RenderStateful() override = default;
			};

			class ResponsiveInterface
			{
			protected:
				ResponsiveInterface() = default;
			public:
				ResponsiveInterface(ResponsiveInterface&& other) = delete;
				ResponsiveInterface& operator=(ResponsiveInterface&& other) = delete;
				ResponsiveInterface(const ResponsiveInterface& other) = delete;
				ResponsiveInterface& operator=(const ResponsiveInterface& other) = delete;

				virtual void response(SDL_Event* event) = 0;

				// MouseEvent
				enum class MouseEvent : uint8_t
				{
					MOUSE_MOTION,
					MOUSE_BUTTON_DOWN,
					MOUSE_BUTTON_UP
				};
				static bool universalMouseEventListen(SDL_Event* event, MouseEvent event_type, const SDL_FRect& rect)
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

				virtual ~ResponsiveInterface() = default;
			};

			class SettableThemePackInterface
			{
			protected:
				SettableThemePackInterface() = default;
			public:
				SettableThemePackInterface(SettableThemePackInterface&& other) = delete;
				SettableThemePackInterface& operator=(SettableThemePackInterface&& other) = delete;
				SettableThemePackInterface(const SettableThemePackInterface& other) = delete;
				SettableThemePackInterface& operator=(const SettableThemePackInterface& other) = delete;

				// virtual void setThemePack(const std::shared_ptr<asset::theme::ThemePack>& theme_pack_pointer) = 0;

				virtual ~SettableThemePackInterface() = default;
			};

			class ChangeableWindowSizeInterface
			{
			protected:
				ChangeableWindowSizeInterface() = default;
			public:
				ChangeableWindowSizeInterface(ChangeableWindowSizeInterface&& other) = delete;
				ChangeableWindowSizeInterface& operator=(ChangeableWindowSizeInterface&& other) = delete;
				ChangeableWindowSizeInterface(const ChangeableWindowSizeInterface& other) = delete;
				ChangeableWindowSizeInterface& operator=(const ChangeableWindowSizeInterface& other) = delete;

				virtual void whenChangedWindowSize(int new_w, int new_h) = 0;

				virtual ~ChangeableWindowSizeInterface() = default;
			};
		}
	}
}