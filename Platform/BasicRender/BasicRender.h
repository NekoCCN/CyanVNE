#pragma once
#include <SDL3/SDL.h>
#include <Platform/WindowContext/WindowContext.h>

namespace cyanvne
{
	namespace platform
	{
		namespace basicrender
		{
			class IRenderable
			{
			protected:
				IRenderable() = default;
			public:
				IRenderable(IRenderable&& other) = delete;
				IRenderable& operator=(IRenderable&& other) = delete;
				IRenderable(const IRenderable& other) = delete;
				IRenderable& operator=(const IRenderable& other) = delete;

				virtual void render(const std::shared_ptr<WindowContext>& window_context) = 0;
				virtual void update(float deltaTime) = 0;
				virtual int getZOrder() const = 0;

				virtual ~IRenderable() = default;
			};

			class IRenderStateful
			{
			protected:
				IRenderStateful() = default;
			public:
				IRenderStateful(IRenderStateful&& other) = delete;
				IRenderStateful& operator=(IRenderStateful&& other) = delete;
				IRenderStateful(const IRenderStateful& other) = delete;
				IRenderStateful& operator=(const IRenderStateful& other) = delete;

				virtual void startRender() = 0;
				virtual void stopRender() = 0;
				virtual void changeRenderStatus(bool status) = 0;
				virtual bool isRender() const = 0;

				virtual ~IRenderStateful() = default;
			};

			class RenderStateful : public IRenderStateful
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

			class ISettableThemePack
			{
			protected:
				ISettableThemePack() = default;
			public:
				ISettableThemePack(ISettableThemePack&& other) = delete;
				ISettableThemePack& operator=(ISettableThemePack&& other) = delete;
				ISettableThemePack(const ISettableThemePack& other) = delete;
				ISettableThemePack& operator=(const ISettableThemePack& other) = delete;

				// virtual void setThemePack(const std::shared_ptr<asset::theme::ThemePack>& theme_pack_pointer) = 0;

				virtual ~ISettableThemePack() = default;
			};

			class IChangeableWindowSize
			{
			protected:
				IChangeableWindowSize() = default;
			public:
				IChangeableWindowSize(IChangeableWindowSize&& other) = delete;
				IChangeableWindowSize& operator=(IChangeableWindowSize&& other) = delete;
				IChangeableWindowSize(const IChangeableWindowSize& other) = delete;
				IChangeableWindowSize& operator=(const IChangeableWindowSize& other) = delete;

				virtual void whenChangedWindowSize(int new_w, int new_h) = 0;

				virtual ~IChangeableWindowSize() = default;
			};

			namespace ResponsiveUtils
			{
				enum class MouseEvent : uint8_t
				{
					MOUSE_MOTION,
					MOUSE_BUTTON_DOWN,
					MOUSE_BUTTON_UP
				};

				bool universalMouseEventListen(const SDL_Event* event, MouseEvent event_type,
					const SDL_FRect& rect);
			}
		}
	}
}